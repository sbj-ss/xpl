/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2021     */
/******************************************/
#ifndef __xplversion_H
#define __xplversion_H

#include "Configuration.h"
#include <stdbool.h>
#include <libxml/tree.h>
#include <libxml/xmlstring.h>
#include <libxpl/xpltree.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Version info */
#define XPL_VERSION_MAJOR 2
#define XPL_VERSION_MINOR 0
#define XPL_VERSION_BETA

#define XPL_VERSION_FLAG_BETA 0x01
#define XPL_VERSION_FLAG_DEBUG 0x02

#ifdef XPL_VERSION_BETA
	#define XPL_VERSION_EFFECTIVE_FLAG_BETA XPL_VERSION_FLAG_BETA
	#define XPL_VERSION_BETA_STRING " beta"
#else
	#define XPL_VERSION_EFFECTIVE_FLAG_BETA 0
	#define XPL_VERSION_BETA_STRING ""
#endif
#ifdef _DEBUG
	#define XPL_VERSION_EFFECTIVE_FLAG_DEBUG XPL_VERSION_FLAG_DEBUG
	#define XPL_VERSION_DEBUG_STRING " (DEBUG)"
#else
	#define XPL_VERSION_EFFECTIVE_FLAG_DEBUG 0
	#define XPL_VERSION_DEBUG_STRING ""
#endif

#define XPL_VERSION (\
	(XPL_VERSION_MAJOR << 16) \
	| (XPL_VERSION_MINOR << 8) \
	| XPL_VERSION_EFFECTIVE_FLAG_BETA \
	| XPL_VERSION_EFFECTIVE_FLAG_DEBUG \
)

#define STRING_VERSION_VALUE(x) #x
#define XPL_VERSION_FULL_M(major, minor) BAD_CAST "C XPL interpreter (codename Polaris) v" \
	STRING_VERSION_VALUE(major) \
	"." \
	STRING_VERSION_VALUE(minor) \
	XPL_VERSION_BETA_STRING \
	XPL_VERSION_DEBUG_STRING \
	" built on " \
	__DATE__
#define XPL_VERSION_FULL XPL_VERSION_FULL_M(XPL_VERSION_MAJOR, XPL_VERSION_MINOR)

XPLPUBFUN int XPLCALL
	xplVersion(void);
XPLPUBFUN xmlChar* XPLCALL
	xplVersionString(void);

typedef enum _xefImplTransport
{
	XEF_IMPL_TRANSPORT_NONE,
	XEF_IMPL_TRANSPORT_WINHTTP,
	XEF_IMPL_TRANSPORT_CURL,
	XEF_IMPL_TRANSPORT_MAX = XEF_IMPL_TRANSPORT_CURL
} xefImplTransport;

typedef enum _xefImplDatabase
{
	XEF_IMPL_DATABASE_NONE,
	XEF_IMPL_DATABASE_ADO,
	XEF_IMPL_DATABASE_ODBC,
	XEF_IMPL_DATABASE_MAX = XEF_IMPL_DATABASE_ODBC
} xefImplDatabase;

typedef enum _xefImplHtmlCleaner
{
	XEF_IMPL_HTML_CLEANER_NONE,
	XEF_IMPL_HTML_CLEANER_TIDY,
	XEF_IMPL_HTML_CLEANER_MAX = XEF_IMPL_HTML_CLEANER_TIDY
} xefImplHtmlCleaner;

typedef enum _xefImplCrypto
{
	XEF_IMPL_CRYPTO_NONE,
	XEF_IMPL_CRYPTO_OPENSSL,
	XEF_IMPL_CRYPTO_MAX = XEF_IMPL_CRYPTO_OPENSSL
} xefImplCrypto;

typedef struct _xefImplementations
{
	xefImplTransport transport;
	xefImplDatabase database;
	xefImplHtmlCleaner html_cleaner;
	xefImplCrypto crypto;
} xefImplementations, *xefImplementationsPtr;

XPLPUBFUN xefImplementationsPtr XPLCALL
	xplGetXefImplementations(void);
XPLPUBFUN xmlNodePtr XPLCALL
	xplXefImplementationsToNodeList(xmlDocPtr doc, xplQName tagname, xefImplementationsPtr impl);
XPLPUBFUN xmlChar* XPLCALL
	xplXefImplementationsToString(xefImplementationsPtr impl);

typedef struct _xplLibraryVersions
{
	xmlChar *curl_version;
	xmlChar *iconv_version;
	xmlChar *idn_version;
	xmlChar *lzma_version;
	xmlChar *odbc_version;
	xmlChar *onig_version;
	xmlChar *ssl_version;
	xmlChar *tidy_version;
	xmlChar *xml2_version;
	xmlChar *yajl_version;
	xmlChar *z_version;
} xplLibraryVersions, *xplLibraryVersionsPtr;

/* These functions return pointers to singletons which caller must not attempt to free */
XPLPUBFUN xplLibraryVersionsPtr XPLCALL
	xplGetCompiledLibraryVersions(void);
XPLPUBFUN xplLibraryVersionsPtr XPLCALL
	xplGetRunningLibraryVersions(void);

XPLPUBFUN xmlNodePtr XPLCALL
	xplLibraryVersionsToNodeList(xmlDocPtr doc, xplQName tagname, xplLibraryVersionsPtr compiled, xplLibraryVersionsPtr running);
XPLPUBFUN xmlChar* XPLCALL
	xplLibraryVersionsToString(xplLibraryVersionsPtr compiled, xplLibraryVersionsPtr running);

XPLPUBFUN bool XPLCALL
	xplInitLibraryVersions(void);
XPLPUBFUN void XPLCALL
	xplCleanupLibraryVersions(void);

#ifdef __cplusplus
}
#endif

#endif
