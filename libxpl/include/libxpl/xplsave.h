/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
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
	xplSafeSerializeDocument(char *filename, xmlDocPtr doc);

/* saves using correct utf-8 paths */
XPLPUBFUN bool XPLCALL
	xplSaveXmlDocToFile(xmlDocPtr doc, xmlChar *filename, char *encoding, int options);

/* for serialization commands */
XPLPUBFUN xmlChar* XPLCALL
	xplSerializeNodeList(xmlNodePtr cur);
XPLPUBFUN xmlChar* XPLCALL
	xplSerializeNodeSet(xmlNodeSetPtr set);

#ifdef __cplusplus
}
#endif
#endif
