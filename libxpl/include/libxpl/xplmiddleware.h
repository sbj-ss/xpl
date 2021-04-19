/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2021     */
/******************************************/
#ifndef __xplwrappers_H
#define __xplwrappers_H

#include "Configuration.h"
#include <libxml/xmlstring.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _xplWrapperMapEntry xplWrapperMapEntry, *xplWrapperMapEntryPtr;

XPLPUBFUN xplWrapperMapEntryPtr XPLCALL
	xplWrapperMapAddEntry(xmlChar *regexString, xmlChar *wrapperFile);
XPLPUBFUN xmlChar* XPLCALL
	xplMapDocWrapper(xmlChar *filename);
XPLPUBFUN bool XPLCALL
	xplInitWrapperMap(void);
XPLPUBFUN void XPLCALL
	xplCleanupWrapperMap(void);

#ifdef __cplusplus
}
#endif
#endif
