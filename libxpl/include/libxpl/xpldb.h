/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2021     */
/******************************************/
#ifndef __xpldb_H
#define __xpldb_H

#include "Configuration.h"
#include <libxml/tree.h>
#include <libxml/xmlstring.h>
#include <libxpl/abstraction/xpr.h>
#include <libxpl/xpltree.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Implementation-agnostic database connections */
typedef void(*xplDBDeallocator)(void* payload);
typedef void*(xplDBConnector)(const xmlChar *conn_string, xmlChar **error);

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
XPLPUBFUN xplDBPtr XPLCALL
	xplGetOrCreateDB(xplDBListPtr list, xplDBConnector connector, xmlChar **error);
XPLPUBFUN void XPLCALL
	xplReleaseDB(xplDBPtr db);

/* Dynaconf functions. Threads must be locked before calling. */
typedef enum _xplDBConfigResult
{
	XPL_DBCR_OK = 0,
	XPL_DBCR_ALREADY_EXISTS = -1,
	XPL_DBCR_INSUFFICIENT_MEMORY = -2,
	XPL_DBCR_CHECK_FAILED = -3,
	XPL_DBCR_NO_PARSER = -4,
	XPL_DBCR_NOT_FOUND = -5
} xplDBConfigResult;

XPLPUBFUN const xmlChar* XPLCALL
	xplDecodeDBConfigResult(xplDBConfigResult result);

XPLPUBFUN xplDBConfigResult XPLCALL
	xplRemoveDB(const xmlChar *name);
XPLPUBFUN xplDBConfigResult XPLCALL
	xplAddDB(const xmlChar *name, const xmlChar *newConnString, bool withCheck);
XPLPUBFUN xplDBConfigResult XPLCALL
	xplChangeDB(const xmlChar *name, const xmlChar *newConnString, bool withCheck);

XPLPUBFUN xmlNodePtr XPLCALL
	xplDatabasesToNodeList(const xmlNodePtr parent, const xplQName qname);

XPLPUBFUN bool XPLCALL
	xplInitDatabases(void);
XPLPUBFUN void XPLCALL
	xplDbGarbageCollect(void);
XPLPUBFUN void XPLCALL
	xplCleanupDatabases(void);

#ifdef __cplusplus
}
#endif
#endif
