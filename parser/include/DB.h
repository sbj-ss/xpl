/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __db_H
#define __db_H

#include "Configuration.h"
#include "Common.h"
#include "abstraction/xpr.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Implementation-agnostic database connections */
typedef void(*xplDBDeallocator)(void* payload);
typedef struct _xplDB xplDB, *xplDBPtr;
struct _xplDB
{
	void* connection;
	bool busy;
	xplDBPtr next;
	xplDBDeallocator dealloc;
};

XPLPUBFUN xplDBPtr XPLCALL
	xplDBCreate(const xplDBPtr aNext, const xplDBDeallocator aDealloc);
XPLPUBFUN void XPLCALL
	xplDBFree(xplDBPtr db);

typedef struct _xplDBList
{
	xmlChar* conn_string;
	XPR_MUTEX lock;
	xplDBPtr first;
	xplDBPtr last; /* speedup */
} xplDBList, *xplDBListPtr;

XPLPUBFUN xplDBListPtr XPLCALL
	xplDBListCreate(const xmlChar *connString);
XPLPUBFUN void XPLCALL
	xplDBListFree(xplDBListPtr dbList);
XPLPUBFUN xplDBListPtr XPLCALL
	xplLocateDBList(const xmlChar *name);
XPLPUBFUN xplDBPtr XPLCALL
	xplLocateAvailDB(xplDBListPtr list);
XPLPUBFUN void XPLCALL
	xplAddDBToDBList(xplDBListPtr list, xplDBPtr db);

/* Dynaconf functions. Threads must be locked before calling. */
XPLPUBFUN void XPLCALL
	xplRemoveDB(xmlChar *name);

typedef enum _xplAddDBResult
{
	XPL_ADD_DB_OK = 0,
	XPL_ADD_DB_ALREADY_EXISTS = -1,
	XPL_ADD_DB_INSUFFICIENT_MEMORY = -2,
	XPL_ADD_DB_CHECK_FAILED = -3,
	XPL_ADD_DB_NO_PARSER = -4
} xplAddDBResult;

XPLPUBFUN xplAddDBResult XPLCALL
	xplAddDB(xmlChar *name, xmlChar *newConnString, bool withCheck);

typedef enum _xplChangeDBResult
{
	XPL_CHANGE_DB_OK = 0,
	XPL_CHANGE_DB_NOT_FOUND = -1,
	XPL_CHANGE_DB_INSUFFICIENT_MEMORY = -2,
	XPL_CHANGE_DB_CHECK_FAILED = -3,
	XPL_CHANGE_DB_NO_PARSER = -4
} xplChangeDBResult;

XPLPUBFUN xplChangeDBResult XPLCALL
	xplChangeDB(xmlChar *name, xmlChar *newConnString, bool withCheck);

XPLPUBFUN xmlNodePtr XPLCALL
	xplDatabasesToNodeList(xmlDocPtr doc, xmlNodePtr parent, const xmlChar *tagName, bool showTags);

XPLPUBFUN bool XPLCALL
	xplReadDatabases(xmlNodePtr cur);

XPLPUBFUN void XPLCALL
	xplCheckDatabases();

XPLPUBFUN void XPLCALL
	xplCleanupDatabases();

#ifdef __cplusplus
}
#endif
#endif
