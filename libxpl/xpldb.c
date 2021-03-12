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

static void _dbListClear(xplDBListPtr cur)
{
	xplDBPtr cur_db, next_db;

	cur_db = cur->first;
	while (cur_db)
	{
		next_db = cur_db->next;
		xplDBFree(cur_db);
		cur_db = next_db;
	}
	cur->first = cur->last = NULL;
}

void xplDBListFree(xplDBListPtr cur)
{
	if (!cur)
		return;
	_dbListClear(cur);
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
	SUCCEED_OR_DIE(xprMutexAcquire(&list->lock));
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
	SUCCEED_OR_DIE(xprMutexRelease(&list->lock));
	return ret;
}

void xplAddDBToDBList(xplDBListPtr list, xplDBPtr db)
{
	if (!list || !db)
		return;
	SUCCEED_OR_DIE(xprMutexAcquire(&list->lock));
	if (list->last)
		list->last->next = db;
	list->last = db;
	if (!list->first)
		list->first = db;
	SUCCEED_OR_DIE(xprMutexRelease(&list->lock));
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

static void dbDeallocator(void *payload, XML_HCBNC xmlChar *name)
{
	UNUSED_PARAM(name);
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
	xplQName tag_name;
	xmlDocPtr doc;
	xmlNodePtr head, tail;
	bool show_tags;
} getDBListContext, *getDBListContextPtr;

static void databaseListScanner(void *payload, void *data, XML_HCBNC xmlChar *name)
{
	getDBListContextPtr ctxt = (getDBListContextPtr) data;
	xplDBListPtr db = (xplDBListPtr) payload;
	xmlNodePtr cur;

	cur = xmlNewDocNode(
		ctxt->doc,
		ctxt->show_tags? NULL: ctxt->tag_name.ns,
		ctxt->show_tags? name: ctxt->tag_name.ncname,
		db->conn_string);
	if (!ctxt->show_tags)
		xmlNewProp(cur, BAD_CAST "name", name);
	APPEND_NODE_TO_LIST(ctxt->head, ctxt->tail, cur);
}

xmlNodePtr xplDatabasesToNodeList(xmlNodePtr parent, const xplQName qname)
{
	getDBListContext ctxt;

	if (!databases)
		return NULL;
	ctxt.doc = parent->doc;
	ctxt.head = ctxt.tail = NULL;
	ctxt.tag_name = qname;
	ctxt.show_tags = !qname.ncname;
	xmlHashScan(databases, databaseListScanner, &ctxt);
	return ctxt.head;
}

bool xplInitDatabases()
{
	xplCleanupDatabases();
	return !!(databases = xmlHashCreate(16));
}

static void _gcScanner(void *payload, void *data, XML_HCBNC xmlChar *name)
{
	UNUSED_PARAM(data);
	UNUSED_PARAM(name);
	_dbListClear((xplDBListPtr) payload);
}

void xplDbGarbageCollect()
{
	if (databases)
		xmlHashScan(databases, _gcScanner, NULL);
}

void xplCleanupDatabases()
{
	if (databases)
	{
		xmlHashFree(databases, dbDeallocator);
		databases = NULL;
	}
}
