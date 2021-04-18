/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2021     */
/******************************************/
#ifndef __xplxjson_H
#define __xplxjson_H

#include "Configuration.h"
#include <libxml/tree.h>
#include <libxml/xmlstring.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JSONX_SCHEMA_URI BAD_CAST "http://www.ibm.com/xmlns/prod/2009/jsonx"

XPLPUBFUN xmlNodePtr XPLCALL
	xplJsonXSerializeNodeList(xmlNodePtr list, bool strictTagNames,	bool valueTypeCheck, bool prettyPrint);
XPLPUBFUN xmlNodePtr XPLCALL
	xplJsonXParse(xmlChar *src, xmlNodePtr parent, bool validateStrings);

#ifdef __cplusplus
}
#endif
#endif
