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

#ifdef __cplusplus
extern "C" {
#endif

/* XPL macro definition */
typedef enum _xplMacroExpansionState
{
	XPL_MACRO_EXPAND_UNKNOWN = -1, /* Only for xplMacroExpansionStateFromString() */
	XPL_MACRO_EXPAND_ALWAYS = 0,
	XPL_MACRO_EXPAND_ONCE,
	XPL_MACRO_EXPANDED,
	XPL_MACRO_EXPAND_NO_DEFAULT /* Can only be an input parameter */
} xplMacroExpansionState;

typedef struct _xplMacro
{
	xmlChar* id;
	xmlNodePtr content;
	int disabled_spin;
	xplMacroExpansionState expansion_state;
	xmlNodePtr node_original_content;
	/* For list-macros */
	xmlChar *name;
	bool ns_is_duplicated;
	xmlNsPtr ns;
	xmlNodePtr parent;
	int line;
	int times_encountered;
	int times_called;
	/* These fields are filled in by the caller */
	xmlNodePtr caller;
	xmlNodePtr return_value;
} xplMacro, *xplMacroPtr;

XPLPUBFUN xplMacroExpansionState XPLCALL
	xplMacroExpansionStateFromString(xmlChar *state, bool allowNoDefault);
XPLPUBFUN xplMacroPtr XPLCALL
	xplMacroCreate(xmlChar *aId, xmlNodePtr aContent, xplMacroExpansionState expansionState);
XMLPUBFUN void XMLCALL
	xplMacroDeallocator(void *payload, const xmlChar *name);
XPLPUBFUN void XPLCALL
	xplMacroFree(xplMacroPtr macro);
XPLPUBFUN xplMacroPtr XPLCALL
	xplMacroCopy(xplMacroPtr macro, xmlNodePtr parent);
XPLPUBFUN void XPLCALL
	xplMacroTableFree(xmlHashTablePtr macros);
XPLPUBFUN xplMacroPtr XPLCALL
	xplMacroLookup(xmlNodePtr element, const xmlChar *href, const xmlChar *name);
XPLPUBFUN xmlChar* XPLCALL
	xplMacroTableToString(xmlNodePtr element, xmlChar* delimiter, bool unique);
XPLPUBFUN xmlNodePtr XPLCALL
	xplMacroToNode(xplMacroPtr macro, xmlChar *tagname, xmlNodePtr parent);
XPLPUBFUN xmlNodePtr XPLCALL
	xplMacroTableToNodeList(xmlNodePtr element, xmlChar *tagQName, bool unique, xmlNodePtr parent);
XPLPUBFUN xmlHashTablePtr XPLCALL
	xplCloneMacroTableUpwards(xmlNodePtr src, xmlNodePtr parent);

#ifdef __cplusplus
}
#endif
#endif
