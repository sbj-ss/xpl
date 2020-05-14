/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/

#ifndef __xplparams_H
#define __xplparams_H

#include "Configuration.h"
#include <stdbool.h>
#include <libxml/tree.h>
#include <libxml/xmlstring.h>
#include <libxpl/abstraction/xpr.h>

#ifdef __cplusplus
extern "C" {
#endif

/* misc */
typedef enum _xplExpectType 
{
	XPL_EXPECT_NUMBER,
	XPL_EXPECT_HEX,
	XPL_EXPECT_STRING,
	XPL_EXPECT_PATH,
	XPL_EXPECT_ANY,
	XPL_EXPECT_UNDEFINED,
	XPL_EXPECT_UNKNOWN
} xplExpectType;

XPLPUBFUN xplExpectType XPLCALL 
	xplExpectTypeFromString(const xmlChar *expect);
XPLPUBFUN xmlChar* XPLCALL
	xplCleanTextValue(xmlChar *data_buf, xplExpectType expect);

typedef enum {
	XPL_PARAM_RES_OK = 0,
	XPL_PARAM_RES_OUT_OF_MEMORY = -1,
	XPL_PARAM_RES_INVALID_INPUT = -2,
	XPL_PARAM_RES_TYPE_CLASH = -3,
	XPL_PARAM_RES_READ_ONLY = -4,
	XPL_PARAM_RES_INTERNAL_ERROR = -5
} xplParamResult;

/* parameter values carrier */
typedef enum 
{
	XPL_PARAM_TYPE_EMPTY = 0,
	XPL_PARAM_TYPE_USERDATA = 1,
	XPL_PARAM_TYPE_FILE = 2,
	XPL_PARAM_TYPE_HEADER = 4
} xplParamType;

XPLPUBFUN int XPLCALL
	xplParamTypeMaskFromString(xmlChar* mask);
XPLPUBFUN bool XPLCALL
	xplParamTypeIsAtomic(xplParamType type);

typedef struct _xplParamFileInfo 
{
	xmlChar *real_path;
	xmlChar *filename;
	int size;
} xplParamFileInfo, *xplParamFileInfoPtr;

/* Входные строковые параметры присваиваются, а не дублируются! */
XPLPUBFUN xplParamFileInfoPtr XPLCALL
	xplParamFileInfoCreate(xmlChar *realPath, xmlChar *filename, int Size);
XPLPUBFUN xplParamFileInfoPtr XPLCALL
	xplParamFileInfoCopy(const xplParamFileInfoPtr src);
XPLPUBFUN void XPLCALL	
	xplParamFileInfoFree(xplParamFileInfoPtr info);

typedef struct _xplParamValues
{
	xplParamType type;
	void **param_tab; /* xmlChar* [] or xplFileInfoPtr [] */
	int param_nr;
	int param_max;
	bool is_locked;
} xplParamValues;
typedef xplParamValues* xplParamValuesPtr;

/* Create new carrier with minimal capacity */
XPLPUBFUN xplParamValuesPtr XPLCALL
	xplParamValuesCreate(void);
/* Copy the carrier and its contents */
XPLPUBFUN xplParamValuesPtr XPLCALL
	xplParamValuesCopy(const xplParamValuesPtr src);
/* Add a value. This function doesn't make a copy of it (use xmlStrdup) */
XPLPUBFUN xplParamResult XPLCALL 
	xplParamValuesAdd(xplParamValuesPtr values, xmlChar *value, xplParamType type);
/* Add a fileinfo item */
XPLPUBFUN xplParamResult XPLCALL
	xplParamValuesAddFileInfo(xplParamValuesPtr values, xmlChar *real_path, xmlChar *filename, int size);
/* Replace all values with the given one. No copy is made, too */
XPLPUBFUN xplParamResult XPLCALL 
	xplParamValuesReplace(xplParamValuesPtr values, xmlChar *value, xplParamType type);
/* Delete the carrier and its contents */
XPLPUBFUN void XPLCALL
	xplParamValuesFree(xplParamValuesPtr values);
/* Make a string of values */
XPLPUBFUN xmlChar* XPLCALL
	xplParamValuesToString(const xplParamValuesPtr values, bool unique, const xmlChar *delim, xplExpectType expect);
/* Make a list of node-packed values */
XPLPUBFUN xmlNodePtr XPLCALL
	xplParamValuesToList(const xplParamValuesPtr values, bool unique, xplExpectType expect, const xmlNsPtr ns, const xmlChar *nodeName, xmlNodePtr parent);

/* user-defined parameters collection */
typedef void* xplParamsPtr;
typedef void (*xplParamsScanner)(void *payload, void *data, xmlChar *name);

XPLPUBFUN xplParamsPtr XPLCALL
	xplParamsCreate(void);
XPLPUBFUN int XPLCALL xplParseParamString(const xmlChar *params, const char *fallbackEncoding, xplParamsPtr ret);
XPLPUBFUN xplParamsPtr XPLCALL
	xplParamsCopy(xplParamsPtr params);
XPLPUBFUN xplParamValuesPtr XPLCALL
	xplParamGet(const xplParamsPtr params, const xmlChar *name);
XPLPUBFUN xmlChar* XPLCALL
	xplParamGetFirstValue(const xplParamsPtr params, const xmlChar *name);
XPLPUBFUN xplParamResult XPLCALL
	xplParamSet(xplParamsPtr params, const xmlChar *name, const xplParamValuesPtr values);
XPLPUBFUN xplParamResult XPLCALL
	xplParamAddValue(xplParamsPtr params, const xmlChar *name, xmlChar *value, xplParamType type);
XPLPUBFUN xplParamResult XPLCALL
	xplParamReplaceValue(xplParamsPtr params, const xmlChar *name, xmlChar *value, xplParamType type);
XPLPUBFUN xplParamResult XPLCALL
	xplParamAddFileInfo(xplParamsPtr params, const xmlChar *name, xmlChar *filename, xmlChar *realPath, int size);
XPLPUBFUN void XPLCALL
	xplParamsScan(xplParamsPtr params, xplParamsScanner f, void *userData);
XPLPUBFUN xmlNodePtr XPLCALL
	xplParamsToList(const xplParamsPtr params, bool unique, xplExpectType expect, const xmlNsPtr ns, const xmlChar *nodeName, xmlNodePtr parent, int typeMask);
XPLPUBFUN void XPLCALL
	xplParamsLockValue(xplParamsPtr params, const xmlChar *name, bool doLock);
XPLPUBFUN void XPLCALL
	xplParamsFree(xplParamsPtr params);

#ifdef __cplusplus
}
#endif
#endif
