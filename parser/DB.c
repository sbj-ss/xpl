#include "DB.h"
#include "Utils.h"
#include "abstraction/ExtFeatures.h"
#include "abstraction/xpr.h"
#include "Messages.h"

static xmlHashTablePtr databases = NULL;
static BOOL db_initialized = FALSE;

xplDBPtr xplDBCreate(const xplDBPtr aNext, const xplDBDeallocator aDealloc)
{
	xplDBPtr db = (xplDBPtr) xmlMalloc(sizeof(xplDB));
	if (!db)
		return NULL;
	memset(db, 0, sizeof(xplDB));
	db->next = aNext;
	db->dealloc = aDealloc;
	return db;
}

void xplDBFree(xplDBPtr cur)
{
	if (cur->dealloc)
		cur->dealloc(cur->connection);
	xmlFree(cur);
}

xplDBListPtr xplDBListCreate(const xmlChar *connString)
{
	xplDBListPtr ret = (xplDBListPtr) xmlMalloc(sizeof(xplDBList));
	if (!ret)
		return NULL;
	memset(ret, 0, sizeof(xplDBList));
	if (!xprMutexInit(&ret->lock))
	{
		xmlFree(ret);
		return NULL;
	}
	ret->conn_string = xmlStrdup(connString);
	return ret;
}

void xplDBListFree(xplDBListPtr cur)
{
	xplDBPtr cur_db, next_db;

	if (!cur)
		return;
	cur_db = cur->first;
	while (cur_db)
	{
		next_db = cur_db->next;
		xplDBFree(cur_db);
		cur_db = next_db;
	}
	xmlFree(cur->conn_string);
	if (!xprMutexCleanup(&cur->lock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	xmlFree(cur);
}

xplDBListPtr xplLocateDBList(const xmlChar *name)
{
	if (!databases)
		return NULL;
	return (xplDBListPtr) xmlHashLookup(databases, name);
}

xplDBPtr xplLocateAvailDB(xplDBListPtr list)
{
	xplDBPtr db, ret = NULL;
	if (!list)
		return NULL;
	if (!xprMutexAcquire(&list->lock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	db = list->first;
	while (db)
	{
		if (!db->busy)
		{
			ret = db;
			break;
		}
		db = db->next;
	}
	if (ret)
		ret->busy = TRUE;
	if (!xprMutexRelease(&list->lock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	return ret;
}

void xplAddDBToDBList(xplDBListPtr list, xplDBPtr db)
{
	if (!list || !db)
		return;
	if (!xprMutexAcquire(&list->lock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	if (list->last)
		list->last->next = db;
	list->last = db;
	if (!list->first)
		list->first = db;
	if (!xprMutexRelease(&list->lock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
}

void xplRemoveDB(xmlChar *name)
{
	xplDBListPtr db = xplLocateDBList(name);
	if (db)
	{
		xplDBListFree(db);
		xmlHashRemoveEntry(databases, name, NULL);
	}
}

xplAddDBResult xplAddDB(xmlChar *name, xmlChar *newConnString, BOOL withCheck)
{
	xplDBListPtr db;

	if (!db_initialized)
		return XPL_ADD_DB_NO_PARSER;
	if (xplLocateDBList(name))
		return XPL_ADD_DB_ALREADY_EXISTS;
	if (withCheck)
	{
		if (!xefDbCheckAvail(newConnString, name, NULL)) // TODO здесь можно воспользоваться сообщением об ошибке
			return XPL_ADD_DB_CHECK_FAILED;
	}
	db = xplDBListCreate(newConnString);
	if (!db)
		return XPL_ADD_DB_INSUFFICIENT_MEMORY;
	xmlHashAddEntry(databases, name, db);
	return XPL_ADD_DB_OK;
}

xplChangeDBResult xplChangeDB(xmlChar *name, xmlChar *newConnString, BOOL withCheck)
{
	xplDBListPtr db, new_db;

	if (!db_initialized)
		return XPL_CHANGE_DB_NO_PARSER;
	db = xplLocateDBList(name);
	if (!db)
		return XPL_CHANGE_DB_NOT_FOUND;
	if (withCheck)
	{
		if (!xefDbCheckAvail(newConnString, name, NULL)) // TODO передать наверх третий параметр
			return XPL_CHANGE_DB_CHECK_FAILED;
	}
	new_db = xplDBListCreate(newConnString);
	if (!new_db)
		return XPL_CHANGE_DB_INSUFFICIENT_MEMORY;
	xplDBListFree(db);
	xmlHashRemoveEntry(databases, name, NULL);
	xmlHashAddEntry(databases, name, new_db);
	return XPL_CHANGE_DB_OK;
}

typedef struct _getDBListContext
{
	xmlChar *tag_name;
	xmlDocPtr doc;
	xmlNsPtr ns;
	xmlNodePtr head, tail;
	BOOL show_tags;
} getDBListContext, *getDBListContextPtr;

static void databaseListScanner(void *payload, void *data, xmlChar *name)
{
	getDBListContextPtr ctxt = (getDBListContextPtr) data;
	xplDBListPtr db = (xplDBListPtr) payload;
	xmlNodePtr cur;

	cur = xmlNewDocNode(ctxt->doc, ctxt->ns, ctxt->show_tags?name:ctxt->tag_name, db->conn_string);
	if (!ctxt->show_tags)
		xmlNewProp(cur, BAD_CAST "Name", name);
	if (ctxt->head)
	{
		ctxt->tail->next = cur;
		cur->prev = ctxt->tail;
		ctxt->tail = cur;
	} else
		ctxt->head = ctxt->tail = cur;
}

xmlNodePtr xplDatabasesToNodeList(xmlDocPtr doc, xmlNodePtr parent, const xmlChar *tagName, BOOL showTags)
{
	getDBListContext ctxt;

	if (!databases)
		return NULL;
	ctxt.doc = doc;
	ctxt.head = NULL;
	EXTRACT_NS_AND_TAGNAME(tagName, ctxt.ns, ctxt.tag_name, parent)
	ctxt.show_tags = showTags;
	xmlHashScan(databases, databaseListScanner, &ctxt);
	return ctxt.head;
}

static void dbDeallocator(void *payload, xmlChar *name)
{
	xplDBListFree((xplDBListPtr) payload);
}

BOOL xplReadDatabases(xmlNodePtr cur)
{
	xplDBListPtr db;
	xmlChar *dbname;

	xplCleanupDatabases();
	databases = xmlHashCreate(16);
	if (!databases)
		return FALSE;

	cur = cur->children;
	while (cur)
	{
		if (cur->type == XML_ELEMENT_NODE)
		{
			if (!xmlStrcmp(cur->name, BAD_CAST "Database"))
			{
				dbname = xmlGetNoNsProp(cur, BAD_CAST "Name");
				if (dbname)
				{
					if (cur->children->type == XML_TEXT_NODE)
					{
						db = xplDBListCreate(cur->children->content);
						xmlHashAddEntry(databases, dbname, (void*) db); /* TODO check for dupes */
					} else
						xplDisplayMessage(xplMsgWarning, BAD_CAST "non-text connection string in config file (line %d), ignored", cur->line);
					xmlFree(dbname);
				} else
					xplDisplayMessage(xplMsgWarning, BAD_CAST "missing dbname attribute in Database element in config file (line %d), ignored", cur->line);
			} else
				xplDisplayMessage(xplMsgWarning, BAD_CAST "unknown node \"%s\" in Databases section in config file (line %d), ignored", cur->line);
		}
		cur = cur->next;
	}
	return TRUE;
}

static void checkDatabase(void *payload, void *data, xmlChar *name)
{
	xmlChar *msg;
	BOOL is_avail = xefDbCheckAvail(((xplDBListPtr) payload)->conn_string, name, &msg);
	if (msg)
	{
		xplDisplayMessage(is_avail? xplMsgInfo: xplMsgWarning, msg);
		xmlFree(msg);
	}
}

void xplCheckDatabases()
{
	xmlHashScan(databases, checkDatabase, NULL);
}

void xplCleanupDatabases()
{
	if (databases)
	{
		xmlHashFree(databases, dbDeallocator);
		databases = NULL;
	}
}
