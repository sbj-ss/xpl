/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __xplxjson_H
#define __xplxjson_H

#include "Configuration.h"
#include <libxml/tree.h>
#include <libxml/xmlstring.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XJSON_SCHEMA_URI BAD_CAST "http://www.ibm.com/xmlns/prod/2009/jsonx"

XPLPUBFUN xmlNodePtr XPLCALL
	xplXJsonSerializeNodeList(xmlNodePtr list, bool strictTagNames,	bool valueTypeCheck);

#ifdef __cplusplus
}
#endif
#endif
