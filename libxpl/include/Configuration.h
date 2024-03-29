/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2021     */
/******************************************/

#ifndef __configuration_H
#define __configuration_H

#include <string.h>
#include <libxml/xmlstring.h>
#include <libxml/xmlversion.h>
/* Calling conventions */
/* _IN_DLL should be set in Makefile or project properties */
#if defined(_MSC_VER) || defined (__MINGW32__)
    // Microsoft and alike
    #define SHARED_EXPORT __declspec(dllexport)
    #define SHARED_IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
    // GCC
    #define SHARED_EXPORT __attribute__((visibility("default")))
    #define SHARED_IMPORT
#else
    // What now?..
    #define SHARED_EXPORT
    #define SHARED_IMPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif

#ifdef _IN_DLL
	#define XPLPUBFUN SHARED_EXPORT
	#define XPLPUBVAR extern SHARED_EXPORT
	#define XPLCALL
#else
	#define XPLPUBFUN SHARED_IMPORT
	#define XPLPUBVAR extern SHARED_IMPORT
	#define XPLCALL
#endif

#define UNUSED_PARAM(x) (void) (x)

/* Macros for enabling compiler-specific checks for printf-like arguments. Borrowed from civetweb. */
#undef PRINTF_FORMAT_STRING
#if defined(_MSC_VER) && _MSC_VER >= 1400
	#include <sal.h>
	#if _MSC_VER > 1400
		#define PRINTF_FORMAT_STRING(s) _Printf_format_string_ s
	#else
		#define PRINTF_FORMAT_STRING(s) __format_string s
	#endif
#else
	#define PRINTF_FORMAT_STRING(s) s
#endif

#ifdef __GNUC__
	#define PRINTF_ARGS(x, y) __attribute__((format(printf, x, y)))
#else
	#define PRINTF_ARGS(x, y)
#endif

/* libxml2 hash callbacks const modifier for the name param */
#if LIBXML_VERSION > 20904
#define XML_HCBNC const
#else
#define XML_HCBNC
#endif

/* Hard-coded params */
/* Should be changed only together with all existing XPL code. */
#define ERROR_NODE_NAME (BAD_CAST "error")
#define ERROR_SOURCE_NAME (BAD_CAST "src")
#define CONTENT_ID_ATTR (BAD_CAST "id")

/* Feature control */
/* Enable tests for two or more subsequent text nodes. Text nodes merging isn't ready for now. */
/* #undef _DEBUG_COALESCING */
/* Enable :crash */
/* #undef _USE_CRASH_COMMAND */
/* Enable :file-op. This command needs to be rewritten. */
/* #undef _FILEOP_SUPPORT */

/* Enable allocated blocks checking for debugging memory leaks.
 * Should be disabled in production as it slows down the interpreter. */
#ifdef _DEBUG
# define _LEAK_DETECTION 
#endif

#ifdef _LEAK_DETECTION
	#define LEAK_DETECTION_PREPARE int __ld_start; int __ld_leaked;
	#define LEAK_DETECTION_START() \
		do { \
			__ld_start = xmlMemBlocks(); \
			__ld_leaked = 0; \
			printf("Leak detection start: %d\n", __ld_start); \
		} while(0)
	#define LEAK_DETECTION_STOP_AND_REPORT() \
		do { \
			__ld_leaked = __ld_start - xmlMemBlocks(); \
			printf("Leak detection end: %d (%d)\n", xmlMemBlocks(), __ld_leaked); \
			if (__ld_leaked) \
			{ \
				printf("Starting memory dump...\n"); \
				xmlMemDisplay(stdout); \
			} \
		} while(0)
	#define LEAK_DETECTION_RET_CODE(old) (__ld_leaked? 18: (old))
	#define XPL_MALLOC(size) xmlMallocLoc((size), __FILE__, __LINE__)
	#define XPL_REALLOC(ptr, size) xmlReallocLoc((ptr), (size), __FILE__, __LINE__)
	#define XPL_STRDUP_NO_CHECK(str) xmlMemStrdupLoc((const char*) (str), __FILE__, __LINE__)
	#define XPL_FREE(ptr) xmlFree((ptr))
#else
	#define LEAK_DETECTION_PREPARE
	#define LEAK_DETECTION_START() (void) 0;
	#define LEAK_DETECTION_STOP_AND_REPORT() (void) 0;
	#define LEAK_DETECTION_RET_CODE(old) (old)
	#define XPL_MALLOC(size) malloc((size))
	#define XPL_REALLOC(ptr, size) realloc((ptr), (size))
	#define XPL_STRDUP_NO_CHECK(str) strdup((str))
	#define XPL_FREE(ptr) free((ptr))
#endif
#define XPL_STRDUP(str) ((str)? XPL_STRDUP_NO_CHECK((str)): NULL)

/* XEF implementation choice */
/* HTML cleaner */
/* #undef _XEF_HTML_CLEANER_TIDY */
/* Database access */
/* #undef _XEF_DB_ADO */
/* #undef _XEF_DB_ODBC */
/* HTTP access */
/* #undef _XEF_TRANSPORT_WINHTTP */
/* Cryptography */
/* #undef _XEF_CRYPTO_OPENSSL */

/* Additional features */
/* Use tcmalloc. Slows down the interpreter on Windows. */
#undef _USE_TCMALLOC
/* Use zlib for compressed XML */
#define _USE_ZLIB
/* Use liblzma in addition to zlib. For compressed XML only. */
#define _USE_LIBLZMA
/* Use libidn. Disabling this will break uri-encode and web-include. */
#define _USE_LIBIDN

#endif
