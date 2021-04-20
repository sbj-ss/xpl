/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2021     */
/******************************************/
#ifndef __xplsave_H
#define __xplsave_H

#include "Configuration.h"
#include <stdbool.h>
#include <stdio.h>
#include <libxml/xmlstring.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#ifdef __cplusplus
extern "C" {
#endif

/* header, elements, props and text only. doesn't use memory manager. */
XPLPUBFUN void XPLCALL
	xplSafeSerializeDocument(const xmlChar *filename, xmlDocPtr doc);

/* saves using correct utf-8 paths */
XPLPUBFUN bool XPLCALL
	xplSaveXmlDocToFile(const xmlDocPtr doc, const xmlChar *filename, const char *encoding, int options);

/* for serialization commands */
XPLPUBFUN xmlChar* XPLCALL
	xplSerializeNodeList(const xmlNodePtr src);
XPLPUBFUN xmlChar* XPLCALL
	xplSerializeNodeSet(const xmlNodeSetPtr set);

#ifdef __cplusplus
}
#endif
#endif
