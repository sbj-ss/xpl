/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __xplwrappers_H
#define __xplwrappers_H

#include "Configuration.h"
#include "Common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _xplDocRole
{
	XPL_DOC_ROLE_UNKNOWN = -1, /* Только для xplDocRoleFromString */
	XPL_DOC_ROLE_PROLOGUE = 0,
	XPL_DOC_ROLE_MAIN,
	XPL_DOC_ROLE_EPILOGUE,
	XPL_DOC_ROLE_MAX = XPL_DOC_ROLE_EPILOGUE
} xplDocRole;

XPLPUBFUN xplDocRole XPLCALL
	xplDocRoleFromString(xmlChar *role);
XPLPUBFUN const xmlChar* XPLCALL
	xplDocRoleToString(xplDocRole role);

typedef enum _xplDocSource
{
	XPL_DOC_SOURCE_ORIGINAL,	/* Перезапись не производилась */
	XPL_DOC_SOURCE_MAPPED,		/* С маппера */
	XPL_DOC_SOURCE_SWEATOUT,	/* Отлуп системы безопасности */
	XPL_DOC_SOURCE_ABSENT,		/* Файла нет */
	XPL_DOC_SOURCE_OVERRIDDEN	/* Была вызвана команда xpl:set-output-document */
} xplDocSource;

XPLPUBFUN const xmlChar* XPLCALL
	xplDocSourceToString(xplDocSource src);

typedef struct _xplWrapperMapEntry xplWrapperMapEntry, *xplWrapperMapEntryPtr;

XPLPUBFUN void XPLCALL
	xplReadWrapperMap(xmlNodePtr cur);
XPLPUBFUN xplWrapperMapEntryPtr XPLCALL
	xplWrapperMapAddEntry(xmlChar *regexString, xmlChar *wrapperFile, xplDocRole role);
XPLPUBFUN xmlChar* XPLCALL
	xplMapDocWrapper(xmlChar *filename, xplDocRole role);
XPLPUBFUN void XPLCALL
	xplCleanupWrapperMap(void);

#ifdef __cplusplus
}
#endif
#endif
