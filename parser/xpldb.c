#include <string.h>
#include <libxpl/abstraction/xpr.h>
#include <libxpl/abstraction/xef.h>
#include <libxpl/xpldb.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>

static xmlHashTablePtr databases = NULL;

xplDBPtr xplDBCreate(const xplDBPtr aNext, const xplDBDeallocator aDealloc)
{
	xplDBPtr db = (xplDBPtr) XPL_MALLOC(sizeof(xplDB));
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
	XPL_FREE(cur);
}

xplDBListPtr xplDBListCreate(const xmlChar *connString)
{
	xplDBListPtr ret = (xplDBListPtr) XPL_MALLOC(sizeof(xplDBList));
	if (!ret)
		return NULL;
	memset(ret, 0, sizeof(xplDBList));
	if (!xprMutexInit(&ret->lock))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		XPL_FREE(ret);
		return NULL;
	}
	ret->conn_string = BAD_CAST XPL_STRDUP((char*) connString);
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
	XPL_FREE(cur->conn_string);
	if (!xprMutexCleanup(&cur->lock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	XPL_FREE(cur);
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
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return NULL;
	}
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
		ret->busy = true;
	if (!xprMutexRelease(&list->lock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	return ret;
}

void xplAddDBToDBList(xplDBListPtr list, xplDBPtr db)
{
	if (!list || !db)
		return;
	if (!xprMutexAcquire(&list->lock)) /* TODO what should we do here? */
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	if (list->last)
		list->last->next = db;
	list->last = db;
	if (!list->first)
		list->first = db;
	if (!xprMutexRelease(&list->lock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
}

const xmlChar* xplDecodeDBConfigResult(xplDBConfigResult result)
{
	switch(result)
	{
		case XPL_DBCR_OK:
			return BAD_CAST "no error";
		case XPL_DBCR_ALREADY_EXISTS:
			return BAD_CAST "database already exists";
		case XPL_DBCR_INSUFFICIENT_MEMORY:
			return BAD_CAST "insufficient memory";
		case XPL_DBCR_CHECK_FAILED:
			return BAD_CAST "couldn't connect to database";
		case XPL_DBCR_NO_PARSER:
			return BAD_CAST "XPL engine not initialized";
		case XPL_DBCR_NOT_FOUND:
			return BAD_CAST "database not known to the engine";
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
			return BAD_CAST "unknown error";
	}
}

static void dbDeallocator(void *payload, xmlChar *name)
{
	xplDBListFree((xplDBListPtr) payload);
}

xplDBConfigResult xplRemoveDB(xmlChar *name)
{
	if (!databases)
		return XPL_DBCR_NO_PARSER;
	if (xmlHashRemoveEntry(databases, name, dbDeallocator))
		return XPL_DBCR_NOT_FOUND;
	return XPL_DBCR_OK;
}

xplDBConfigResult xplAddDB(xmlChar *name, xmlChar *newConnString, bool withCheck)
{
	xplDBListPtr db;

	if (!databases)
		return XPL_DBCR_NO_PARSER;
	if (xplLocateDBList(name))
		return XPL_DBCR_ALREADY_EXISTS;
	if (withCheck)
	{
		if (!xefDbCheckAvail(newConnString, name, NULL))
			return XPL_DBCR_CHECK_FAILED;
	}
	db = xplDBListCreate(newConnString);
	if (!db)
		return XPL_DBCR_INSUFFICIENT_MEMORY;
	if (xmlHashAddEntry(databases, name, db))
	{
		xplDBListFree(db);
		return XPL_DBCR_INSUFFICIENT_MEMORY;
	}
	return XPL_DBCR_OK;
}

xplDBConfigResult xplChangeDB(xmlChar *name, xmlChar *newConnString, bool withCheck)
{
	xplDBListPtr db, new_db;

	if (!databases)
		return XPL_DBCR_NO_PARSER;
	db = xplLocateDBList(name);
	if (!db)
		return XPL_DBCR_NOT_FOUND;
	if (withCheck)
	{
		if (!xefDbCheckAvail(newConnString, name, NULL))
			return XPL_DBCR_CHECK_FAILED;
	}
	new_db = xplDBListCreate(newConnString);
	if (!new_db)
		return XPL_DBCR_INSUFFICIENT_MEMORY;
	if (xmlHashUpdateEntry(databases, name, new_db, dbDeallocator))
	{
		xplDBListFree(new_db);
		return XPL_DBCR_INSUFFICIENT_MEMORY;
	}
	return XPL_DBCR_OK;
}

typedef struct _getDBListContext
{
	xmlChar *tag_name;
	xmlDocPtr doc;
	xmlNsPtr ns;
	xmlNodePtr head, tail;
	bool show_tags;
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

xmlNodePtr xplDatabasesToNodeList(xmlNodePtr parent, const xmlChar *tagName, bool showTags)
{
	getDBListContext ctxt;

	if (!databases)
		return NULL;
	ctxt.doc = parent->doc;
	ctxt.head = NULL;
	EXTRACT_NS_AND_TAGNAME(tagName, ctxt.ns, ctxt.tag_name, parent)
	ctxt.show_tags = showTags;
	xmlHashScan(databases, databaseListScanner, &ctxt);
	return ctxt.head;
}

bool xplReadDatabases(xmlNodePtr cur, bool warningsAsErrors)
{
	xplDBListPtr db;
	xmlChar *dbname;
	bool ok = true;
	const xplMsgType msg_type = warningsAsErrors? xplMsgError: xplMsgWarning;
	const xmlChar *tail = BAD_CAST (warningsAsErrors? "stopping": "ignored");

	xplCleanupDatabases();
	databases = xmlHashCreate(16);
	if (!databases)
		return false;

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
						if (xmlHashAddEntry(databases, dbname, (void*) db))
						{
							xplDisplayMessage(msg_type, BAD_CAST "duplicate database name in config file (line %d), %s", cur->line, tail);
							ok = false;
						}
					} else {
						xplDisplayMessage(msg_type, BAD_CAST "non-text connection string in config file (line %d), %s", cur->line, tail);
						ok = false;
					}
					XPL_FREE(dbname);
				} else {
					xplDisplayMessage(msg_type, BAD_CAST "missing dbname attribute in Database element in config file (line %d), %s", cur->line, tail);
					ok = false;
				}
			} else {
				xplDisplayMessage(msg_type, BAD_CAST "unknown node \"%s\" in Databases section in config file (line %d), %s", cur->name, cur->line, tail);
				ok = false;
			}
		}
		cur = cur->next;
	}
	return !warningsAsErrors || ok;
}

static void checkDatabase(void *payload, void *data, xmlChar *name)
{
	xmlChar *msg;
	bool is_avail = xefDbCheckAvail(((xplDBListPtr) payload)->conn_string, name, &msg);
	if (msg)
	{
		xplDisplayMessage(is_avail? xplMsgInfo: xplMsgWarning, msg);
		XPL_FREE(msg);
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
