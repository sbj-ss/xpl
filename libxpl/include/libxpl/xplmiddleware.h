/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2021     */
/******************************************/
#ifndef __xplwrappers_H
#define __xplwrappers_H

#include "Configuration.h"
#include <libxml/tree.h>
#include <libxml/xmlstring.h>
#include <libxpl/xpltree.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _xplMWResult
{
	XPL_MW_OK,
	XPL_MW_ALREADY_EXISTS,
	XPL_MW_NOT_FOUND,
	XPL_MW_BAD_REGEX,
	XPL_MW_OUT_OF_MEMORY
} xplMWResult;

XPLPUBFUN const xmlChar* XPLCALL
	xplMWResultToString(xplMWResult res);
XPLPUBFUN xplMWResult XPLCALL
	xplMWAddEntry(const xmlChar *regexString, const xmlChar *wrapperFile, bool allowReplace);
XPLPUBFUN xplMWResult XPLCALL
	xplMWChangeEntry(const xmlChar *regexString, const xmlChar *wrapperFile, bool allowCreate);
XPLPUBFUN xplMWResult XPLCALL
	xplMWRemoveEntry(const xmlChar *regexString, bool ignoreMissing);
XPLPUBFUN xplMWResult XPLCALL
	xplMWClear(void);
XPLPUBFUN xmlNodePtr XPLCALL
	xplMWListEntries(const xmlDocPtr doc, const xplQName qname);
XPLPUBFUN const xmlChar* XPLCALL
	xplMWGetWrapper(const xmlChar *filename);
XPLPUBFUN bool XPLCALL
	xplInitMiddleware(void);
XPLPUBFUN void XPLCALL
	xplCleanupMiddleware(void);

#ifdef __cplusplus
}
#endif
#endif
