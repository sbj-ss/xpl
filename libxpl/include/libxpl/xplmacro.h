/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __xplmacro_H
#define __xplmacro_H

#include "Configuration.h"
#include <stdbool.h>
#include <libxml/xmlstring.h>
#include <libxml/tree.h>
#include <libxpl/xplcommand.h>
#include <libxpl/xpltree.h>

#ifdef __cplusplus
extern "C" {
#endif

/* XPL macro definition */
typedef enum _xplMacroExpansionState
{
	XPL_MACRO_EXPAND_UNKNOWN = -1, /* Only for xplMacroExpansionStateFromString() */
	XPL_MACRO_EXPAND_ALWAYS = 0,
	XPL_MACRO_EXPAND_ONCE,
	XPL_MACRO_EXPANDED
} xplMacroExpansionState;

typedef struct _xplMacro
{
	xmlChar* id;
	xmlNodePtr content;
	int disabled_spin;
	xplMacroExpansionState expansion_state;
	xmlNodePtr node_original_content;
	/* For list-macros */
	xplQName qname;
	bool ns_is_duplicated;
	xmlNodePtr parent;
	int line;
	int times_encountered;
	int times_called;
	/* These fields are filled in by the caller */
	xmlNodePtr caller;
	xmlNodePtr return_value;
} xplMacro, *xplMacroPtr;

XPLPUBFUN xplMacroExpansionState XPLCALL
	xplMacroExpansionStateFromString(const xmlChar *state);
XPLPUBFUN xmlChar* XPLCALL
	xplMacroExpansionStateGetter(xplCommandInfoPtr commandInfo, const xmlChar *expect, xplMacroExpansionState *result);
XPLPUBFUN xplMacroPtr XPLCALL
	xplMacroCreate(xmlChar *aId, xmlNodePtr aContent, xplMacroExpansionState expansionState);
XPLPUBFUN void XPLCALL
	xplMacroFree(xplMacroPtr macro);
XPLPUBFUN xplMacroPtr XPLCALL
	xplMacroCopy(xplMacroPtr macro, xmlNodePtr parent);
XPLPUBFUN void XPLCALL
	xplMacroTableFree(xmlHashTablePtr macros);
XPLPUBFUN xplMacroPtr XPLCALL
	xplMacroGetFromHashByElement(xmlHashTablePtr hash, xmlNodePtr element);
XPLPUBFUN xplMacroPtr XPLCALL
	xplMacroGetFromHashByQName(xmlHashTablePtr hash, xplQName qname);
XPLPUBFUN void XPLCALL
	xplMacroAddToHash(xmlHashTablePtr hash, xplMacroPtr macro);
XPLPUBFUN xplMacroPtr XPLCALL
	xplMacroLookupByElement(xmlNodePtr carrier, xmlNodePtr element);
XPLPUBFUN xplMacroPtr XPLCALL
	xplMacroLookupByQName(xmlNodePtr carrier, xplQName qname);
XPLPUBFUN xmlChar* XPLCALL
	xplMacroTableToString(xmlNodePtr element, xmlChar* delimiter, bool unique);
XPLPUBFUN xmlNodePtr XPLCALL
	xplMacroToNode(xplMacroPtr macro, xplQName tagname, xmlNodePtr parent);
XPLPUBFUN xmlNodePtr XPLCALL
	xplMacroTableToNodeList(xmlNodePtr element, xplQName tagname, bool unique, xmlNodePtr parent);
XPLPUBFUN xmlHashTablePtr XPLCALL
	xplCloneMacroTableUpwards(xmlNodePtr src, xmlNodePtr parent);

#ifdef __cplusplus
}
#endif
#endif
