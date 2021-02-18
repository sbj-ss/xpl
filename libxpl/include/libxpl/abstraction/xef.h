/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __xef_H
#define __xef_H

#include "Configuration.h"
#include <stdbool.h>
#include <libxml/xmlstring.h>
#include <libxpl/xpldb.h>

#ifdef __cplusplus
extern "C" {
#endif

/* XEF (XPL External Features) is an abstraction layer for certain things like regular expressions,
 * database connections etc that may have multiple implementations on different platforms.
 * No implementation-specific headers should be mentioned in this file - it only defines
 * implementation-agnostic APIs.
 */

/* Start/stop all enabled features. Not thread-safe. */
typedef struct _xefStartupParams
{
	/* output */
	xmlChar *error;		/* must be freed if not NULL */
} xefStartupParams, *xefStartupParamsPtr;

XPLPUBFUN bool XPLCALL
	xefStartup(xefStartupParamsPtr params);
XPLPUBFUN bool XPLCALL
	xefIsStarted(void);
XPLPUBFUN void XPLCALL
	xefShutdown(void);

/* regular expressions will be added here later */

/* High-level transport protocols (mostly HTTP(S)? for now) */
typedef struct _xefFetchDocumentParams
{
	/* input */
	xmlChar *uri;
	xmlChar *extra_query;		/* POST data etc */
	/* output */
	xmlChar *encoding;			/* must be freed */
	xmlChar *document;			/* must be freed. May contain zeros inside */
	size_t document_size;		/* in bytes including terminating zero */
	xmlChar *real_uri;			/* must be freed */
	int status_code;			/* may be undefined due to errors before actual fetching */
	xmlChar *error;				/* must be freed */
} xefFetchDocumentParams, *xefFetchDocumentParamsPtr;

XPLPUBFUN bool XPLCALL
	xefFetchDocument(xefFetchDocumentParamsPtr params);
/* clears only output parameters */
XPLPUBFUN void XPLCALL
	xefFetchParamsClear(xefFetchDocumentParamsPtr params);

/* databases */
XPLPUBFUN void XPLCALL
	xefDbDeallocateDb(void *db_handle);
XPLPUBFUN bool XPLCALL
	xefDbCheckAvail(const xmlChar* connString, const xmlChar *name, xmlChar **msg);

typedef enum _xefDbStreamType 
{
	XEF_DB_STREAM_UNKNOWN = -1,
	XEF_DB_STREAM_TDS,
	XEF_DB_STREAM_XML
} xefDbStreamType;

typedef struct _xefDbField 
{
	xmlChar *name;
	xmlChar *value;
	bool is_null;
	size_t value_size; /* in bytes without null terminator */
	bool needs_copy;
	void *db_object; /* implementation specific */
} xefDbField, *xefDbFieldPtr;

typedef struct _xefDbRow 
{
	int field_count;
	xefDbFieldPtr fields;
} xefDbRow, *xefDbRowPtr;

/* implementation is opaque */
typedef struct xefDbContext *xefDbContextPtr;

typedef bool (*xefDbGetRowCallback)(xefDbRowPtr row, void *userData);
/* decouple with xpldb.h */
typedef struct _xplDBList xplDBList, *xplDBListPtr;

typedef struct _xefDbQueryParams 
{
	/* input */
	xefDbStreamType desired_stream_type;
	bool cleanup_nonprintable;
	xmlChar *query;
	xplDBListPtr db_list;
	void *user_data;
	/* output */
	xmlChar *error;		/* must be freed */
} xefDbQueryParams, *xefDbQueryParamsPtr;

XPLPUBFUN xefDbRowPtr XPLCALL
	xefDbGetRow(xefDbContextPtr ctxt);
XPLPUBFUN xmlChar* XPLCALL
	xefDbGetError(xefDbContextPtr ctxt);
XPLPUBFUN void* XPLCALL
	xefDbGetUserData(xefDbContextPtr ctxt);
XPLPUBFUN bool XPLCALL
	xefDbHasRecordset(xefDbContextPtr ctxt);
XPLPUBFUN bool XPLCALL
	xefDbGetStreamType(xefDbContextPtr ctxt);
XPLPUBFUN ssize_t XPLCALL
	xefDbGetRowCount(xefDbContextPtr ctxt);

XPLPUBFUN xefDbContextPtr XPLCALL
	xefDbQuery(xefDbQueryParamsPtr params);
XPLPUBFUN bool XPLCALL
	xefDbNextRowset(xefDbContextPtr ctxt);
XPLPUBFUN void XPLCALL 
	xefDbEnumRows(xefDbContextPtr ctxt, xefDbGetRowCallback cb, void *user_data);
XPLPUBFUN xmlChar* XPLCALL
	xefDbAccessStreamData(xefDbContextPtr ctxt, size_t *size);
XPLPUBFUN void XPLCALL
	xefDbUnaccessStreamData(xefDbContextPtr ctxt, xmlChar *data);
XPLPUBFUN void XPLCALL
	xefDbFreeContext(xefDbContextPtr ctxt);
XPLPUBFUN void XPLCALL
	xefDbFreeParams(xefDbQueryParamsPtr params, bool freeCarrier);

/* HTML -> XHTML */
typedef struct _xefCleanHtmlParams
{
	/* input */
	xmlChar *document;			/* must be UTF-8 encoded */
	/* output */
	xmlChar *clean_document;	/* must be freed */
	size_t clean_document_size;
	xmlChar *error;				/* must be freed */
} xefCleanHtmlParams, *xefCleanHtmlParamsPtr;

XPLPUBFUN bool XPLCALL
	xefCleanHtml(xefCleanHtmlParamsPtr params);

/* sanity checks */
#ifdef _XEF_HTML_CLEANER_TIDY
	#ifdef _XEF_HAS_HTML_CLEANER
		#error XEF: another html cleaner is used already
	#else
		#define _XEF_HAS_HTML_CLEANER
	#endif
#endif

#ifndef _XEF_HAS_HTML_CLEANER
	#pragma message("WARNING: no HTML cleaner is used, some xpl:include features will be unavailable")
#endif

#ifdef _XEF_TRANSPORT_WINHTTP
	#ifdef _XEF_HAS_TRANSPORT
		#error XEF: another document transport is used already
	#else
		#define _XEF_HAS_TRANSPORT
	#endif
#endif

#ifdef _XEF_TRANSPORT_CURL
	#ifdef _XEF_HAS_TRANSPORT
		#error XEF: another document transport is used already
	#else
		#define _XEF_HAS_TRANSPORT
	#endif
#endif

#ifndef _XEF_HAS_TRANSPORT
	#pragma message("WARNING: no external document transport is used, some xpl:include features will be unavailable")
#endif

#ifdef _XEF_DB_ADO
	#ifdef _XEF_HAS_DB
		#error XEF: another database connector is used already
	#else
		#define _XEF_HAS_DB
	#endif
#endif

#ifdef _XEF_DB_ODBC
	#ifdef _XEF_HAS_DB
		#error XEF: another database connector is used already
	#else
		#define _XEF_HAS_DB
	#endif
#endif

#ifndef _XEF_HAS_DB
	#pragma message("WARNING: no database module, xpl:sql is unusable")
#endif

#ifdef _XEF_HAS_REGEX
bool xefStartupRegex(xefStartupParamsPtr params);
void xefShutdownRegex(void);
#endif

#ifdef _XEF_HAS_TRANSPORT
bool xefStartupTransport(xefStartupParamsPtr params);
void xefShutdownTransport(void);
#endif

#ifdef _XEF_HAS_DB
bool xefStartupDatabase(xefStartupParamsPtr params);
void xefShutdownDatabase(void);
#endif

#ifdef _XEF_HAS_HTML_CLEANER
bool xefStartupHtmlCleaner(xefStartupParamsPtr params);
void xefShutdownHtmlCleaner(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
