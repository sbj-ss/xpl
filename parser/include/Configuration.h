/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/

#ifndef __configuration_H
#define __configuration_H

#include <libxml/xmlstring.h>
/* Calling conventions */
/* _IN_DLL should be set in Makefile or project properties */
#if defined(_MSC_VER)
    // Microsoft
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
	#define XPLPUBVAR SHARED_IMPORT
	#define XPLCALL
#endif

#ifdef __GNUC__
	#define HAVE_DESIGNATED_INITIALIZERS
#endif

#ifdef HAVE_DESIGNATED_INITIALIZERS
	#define SFINIT(f, ...) f = __VA_ARGS__
#else
	#define SFINIT(f, ...) __VA_ARGS__
#endif

/* Bool basics */
#ifndef TRUE
	#define TRUE 1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

#ifndef BOOL
	#define BOOL int
#endif

/* Hard-coded params */
/* Should be changed only together with all existing XPL code. */
#define ERROR_NODE_NAME (BAD_CAST "error")
#define ERROR_SOURCE_NAME (BAD_CAST "src")

/* Feature control */
/* Enable tests for two or more subsequent text nodes. Text nodes merging isn't ready for now. */
#undef _DEBUG_COALESCING
/* Enable :set-option, :rehash, :restart, :xx-db, :xx-sa-mode. Not now. */
/* #define _DYNACONF_SUPPORT */
/* Enable threading support */
/* #define _THREADING_SUPPORT */
/* Enable automatic restarts on crash. Session isn't saved for now so it's better to keep this off. */
/* #define _USE_PHOENIX_TECH */
/* Enable :crash (for PhoenixTech debugging) */
/* #define _USE_CRASH_COMMAND */
/* Enable :file-op. This command needs to be rewritten. */
/* #define _FILEOP_SUPPORT */

/* Enable allocated blocks checking for debugging memory leaks.
 * Should be disabled in production as it slows down the interpreter. */
#ifdef _DEBUG
# define _LEAK_DETECTION 
#endif

#ifdef _LEAK_DETECTION
# define LEAK_DETECTION_PREPARE int __ld_start;
# define LEAK_DETECTION_START  __ld_start = xmlMemBlocks(); printf("Leak detection start: %d\n", __ld_start);
# define LEAK_DETECTION_STOP   printf("Leak detection end: %d (%d)\n", xmlMemBlocks(), __ld_start - xmlMemBlocks());
#else
# define LEAK_DETECTION_PREPARE
# define LEAK_DETECTION_START
# define LEAK_DETECTION_STOP
#endif

/* XEF implementation choice */
/* HTML cleaner */
#define _XEF_HTML_CLEANER_TIDY
/* HTTP access */
#undef _XEF_TRANSPORT_WINHTTP
/* Database access */
#undef _XEF_DB_ADO
#define _XEF_DB_ODBC

/* Additional features */
/* Use tcmalloc. Slows down the interpreter on Windows. */
#undef _USE_TCMALLOC
/* Use liblzma in addition to zlib. For compressed XML only. */
#define _USE_LZMA
/* Use libidn. Disabling this will break uri-encode and web-include. */
#define _USE_LIBIDN

/* Version info */
#define XPL_VERSION_MAJOR 1
#define XPL_VERSION_MINOR 5
#define XPL_VERSION ((XPL_VERSION_MAJOR << 8) | XPL_VERSION_MINOR)
#define XPL_VERSION_BETA 1

#ifdef XPL_VERSION_BETA
# define XPL_VERSION_BETA_STRING " beta"
#else
# define XPL_VERSION_BETA_STRING ""
#endif

#ifdef _DEBUG
#define XPL_VERSION_DEBUG_STRING " (DEBUG)"
#else
#define  XPL_VERSION_DEBUG_STRING ""
#endif

#define STRING_VERSION_VALUE(x) #x
#define XPL_VERSION_FULL_M(major, minor) BAD_CAST "C XPL interpreter (codename Polaris) v " \
	STRING_VERSION_VALUE(major) \
	"." \
	STRING_VERSION_VALUE(minor) \
	XPL_VERSION_BETA_STRING \
	XPL_VERSION_DEBUG_STRING \
	" built on " \
	__DATE__
#define XPL_VERSION_FULL XPL_VERSION_FULL_M(XPL_VERSION_MAJOR, XPL_VERSION_MINOR)

#endif
