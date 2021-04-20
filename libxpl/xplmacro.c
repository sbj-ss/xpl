#include <stdio.h>
#include <string.h>
#include <libxpl/xplmacro.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

xplMacroExpansionState xplMacroExpansionStateFromString(const xmlChar *state)
{
	if (!state)
		return XPL_MACRO_EXPAND_ALWAYS; /* default */
	if (!xmlStrcasecmp(state, BAD_CAST "true") || !xmlStrcasecmp(state, BAD_CAST "now"))
		return XPL_MACRO_EXPANDED;
	else if (!xmlStrcasecmp(state, BAD_CAST "false") || !xmlStrcasecmp(state, BAD_CAST "always"))
		return XPL_MACRO_EXPAND_ALWAYS;
	else if (!xmlStrcasecmp(state, BAD_CAST "once"))
		return XPL_MACRO_EXPAND_ONCE;
	else
		return XPL_MACRO_EXPAND_UNKNOWN;
}

xmlChar* xplMacroExpansionStateGetter(xplCommandInfoPtr info, const xmlChar *raw_value, xplMacroExpansionState *result)
{
	UNUSED_PARAM(info);
	*result = xplMacroExpansionStateFromString(raw_value);
	if (*result == XPL_MACRO_EXPAND_UNKNOWN)
		return xplFormat("invalid expand value '%s'", raw_value);
	return NULL;
}

xplMacroPtr xplMacroCreate(const xmlChar *aId, xmlNodePtr aContent, xplMacroExpansionState expansionState)
{
	xplMacroPtr macro = (xplMacroPtr) XPL_MALLOC(sizeof(xplMacro));
	if (!macro)
		return NULL;
	memset(macro, 0, sizeof(xplMacro));
	macro->id = BAD_CAST XPL_STRDUP((char*) aId);
	macro->disabled_spin = 0;
	macro->content = aContent;
	macro->expansion_state = expansionState;
	return macro;
}

void xplMacroFree(xplMacroPtr macro)
{
	if (!macro)
		return;
	if (macro->id)
		XPL_FREE(macro->id);
	if (macro->content)
		xmlFreeNodeList(macro->content);
	if (macro->ns_defs)
		xmlFreeNsList(macro->ns_defs);
	if (macro->qname.ncname)
		XPL_FREE(macro->qname.ncname);
	if (macro->qname.ns && macro->ns_is_duplicated)
		xmlFreeNs(macro->qname.ns);
	XPL_FREE(macro);
}

xplMacroPtr xplMacroCopy(const xplMacroPtr macro, const xmlNodePtr parent)
{
	xplMacroPtr ret;

	if (!macro)
		return NULL;
	ret = xplMacroCreate(macro->id, xplCloneNodeList(macro->content, parent, parent->doc), macro->expansion_state);
	if (!ret)
		return NULL;
	ret->qname.ncname = BAD_CAST XPL_STRDUP((char*) macro->qname.ncname);
	ret->ns_is_duplicated = true;
	ret->qname.ns = xmlCopyNamespace(macro->qname.ns);
	ret->line = -1;
	ret->parent = parent;
	return ret;
}

static void _macroDeallocator(void *payload, XML_HCBNC xmlChar *name)
{
	UNUSED_PARAM(name);
	xplMacroFree((xplMacroPtr) payload);
}

void xplMacroTableFree(xmlHashTablePtr macros)
{
	if (macros)
		xmlHashFree(macros, _macroDeallocator);
}

xplMacroPtr xplMacroGetFromHashByElement(const xmlHashTablePtr hash, const xmlNodePtr element)
{
	return xmlHashLookup2(hash, element->name, element->ns? element->ns->href: NULL);
}

xplMacroPtr xplMacroGetFromHashByQName(const xmlHashTablePtr hash, const xplQName qname)
{
	return xmlHashLookup2(hash, qname.ncname, qname.ns? qname.ns->href: NULL);
}

void xplMacroAddToHash(xmlHashTablePtr hash, xplMacroPtr macro)
{
	xmlHashUpdateEntry2(hash, macro->qname.ncname, macro->qname.ns? macro->qname.ns->href: NULL, macro, _macroDeallocator);
}

xplMacroPtr xplMacroLookupByElement(const xmlNodePtr carrier, const xmlNodePtr element)
{
	xplMacroPtr macro = NULL;
	xmlNodePtr cur = carrier;

	while (!macro && cur && (cur->type == XML_ELEMENT_NODE))
	{
		if (cur->_private)
			macro = (xplMacroPtr) xplMacroGetFromHashByElement((xmlHashTablePtr) cur->_private, element);
		cur = cur->parent;
	}
	return macro;
}

xplMacroPtr xplMacroLookupByQName(const xmlNodePtr carrier, const xplQName qname)
{
	xplMacroPtr macro = NULL;
	xmlNodePtr cur = carrier;

	while (!macro && cur && (cur->type == XML_ELEMENT_NODE))
	{
		if (cur->_private)
			macro = (xplMacroPtr) xplMacroGetFromHashByQName((xmlHashTablePtr) cur->_private, qname);
		cur = cur->parent;
	}
	return macro;
}

typedef struct _macroStringScannerCtxt
{
	xmlHashTablePtr unique_hash;
	int count;
	size_t len;
	xmlChar *cur;
	const xmlChar *delimiter;
	size_t delimiter_len;
} macroStringScannerCtxt, *macroStringScannerCtxtPtr;

static void _macroCountScanner(void *payload, void *data, XML_HCBNC xmlChar *name)
{
	macroStringScannerCtxtPtr ctxt = (macroStringScannerCtxtPtr) data;
	xplMacroPtr macro = (xplMacroPtr) payload;

	if (ctxt->unique_hash && xplMacroGetFromHashByQName(ctxt->unique_hash, macro->qname))
		return;
	ctxt->count++;
	ctxt->len += xmlStrlen(name);
	if (macro->qname.ns)
	{
		ctxt->len++; /* ":" */
		ctxt->len += xmlStrlen(macro->qname.ns->prefix);
	}
	if (ctxt->unique_hash)
		xplMacroAddToHash(ctxt->unique_hash, macro);
}

static void _macroStringScanner(void *payload, void *data, XML_HCBNC xmlChar *name)
{
	macroStringScannerCtxtPtr ctxt = (macroStringScannerCtxtPtr) data;
	xplMacroPtr macro = (xplMacroPtr) payload;
	int len;

	if (ctxt->unique_hash && xplMacroGetFromHashByQName(ctxt->unique_hash, macro->qname))
		return;
	if (macro->qname.ns)
	{
		len = xmlStrlen(macro->qname.ns->prefix);
		memcpy(ctxt->cur, macro->qname.ns->prefix, len);
		ctxt->cur += len;
		*(ctxt->cur) = ':';
		ctxt->cur++;
	}
	len = xmlStrlen(name);
	memcpy(ctxt->cur, name, len);
	ctxt->cur += len;
	if (ctxt->delimiter)
	{
		strcpy((char*) ctxt->cur, (char*) ctxt->delimiter);
		ctxt->cur += ctxt->delimiter_len;
	}
	if (ctxt->unique_hash)
		xplMacroAddToHash(ctxt->unique_hash, macro);
}

xmlChar* xplMacroTableToString(const xmlNodePtr element, const xmlChar* delimiter, bool unique)
{
	xmlNodePtr cur;
	macroStringScannerCtxt ctxt;
	xmlChar *ret;
	size_t ret_len;

	if (!element)
		return NULL;
	if (unique)
		ctxt.unique_hash = xmlHashCreate(cfgInitialMacroTableSize * 2);
	else
		ctxt.unique_hash = NULL;
	ctxt.count = 0;
	ctxt.len = 0;
	cur = element;
	while (cur && (cur->type == XML_ELEMENT_NODE))
	{
		if (cur->_private)
			xmlHashScan((xmlHashTablePtr) cur->_private, _macroCountScanner, &ctxt);
		cur = cur->parent;
	}
	if (!ctxt.count)
		return NULL;
	if (delimiter)
		ctxt.delimiter_len = xmlStrlen(delimiter);
	else
		ctxt.delimiter_len = 0;
	ret_len = ctxt.len + ctxt.count*ctxt.delimiter_len + 1;
	ret = (xmlChar*) XPL_MALLOC(ret_len);
	if (!ret)
		return NULL;
	ctxt.cur = ret;
	ctxt.delimiter = delimiter;
	if (unique)
	{
		xmlHashFree(ctxt.unique_hash, NULL);
		ctxt.unique_hash = xmlHashCreate(cfgInitialMacroTableSize * 2);
	}
	cur = element;
	while (cur && (cur->type == XML_ELEMENT_NODE))
	{
		if (cur->_private)
			xmlHashScan((xmlHashTablePtr) cur->_private, _macroStringScanner, &ctxt);
		cur = cur->parent;
	}
	if (unique)
		xmlHashFree(ctxt.unique_hash, NULL);
	ctxt.cur -= ctxt.delimiter_len; /* skip last delimiter */
	*ctxt.cur = 0;
	return ret;
}

xmlNodePtr xplMacroToNode(const xplMacroPtr macro, const xplQName tagname, const xmlNodePtr parent)
{
	xmlNodePtr ret;
	xmlChar *expansion_state;
	char num_buf[12];
	xmlChar *parent_name = NULL;

	if (!macro || !parent)
		return NULL;
	ret = xmlNewDocNode(parent->doc, tagname.ns, tagname.ncname, NULL);
	xmlNewProp(ret, BAD_CAST "namespaceuri", macro->qname.ns? macro->qname.ns->href: NULL);
	xmlNewProp(ret, BAD_CAST "prefix", macro->qname.ns? macro->qname.ns->prefix: NULL);
	xmlNewProp(ret, BAD_CAST "name", macro->qname.ncname);
	snprintf(num_buf, sizeof(num_buf), "%d", macro->line);
	xmlNewProp(ret, BAD_CAST "line", BAD_CAST num_buf);
	if (macro->parent->ns && macro->parent->ns->href)
	{
		parent_name = BAD_CAST XPL_STRDUP((char*) macro->parent->ns->prefix);
		parent_name = xmlStrcat(parent_name, BAD_CAST ":");
	}
	parent_name = xmlStrcat(parent_name, macro->parent->name);
	xmlNewProp(ret, BAD_CAST "parentname", parent_name);
	XPL_FREE(parent_name);
	snprintf(num_buf, sizeof(num_buf), "%d", macro->parent->line);
	xmlNewProp(ret, BAD_CAST "parentline", BAD_CAST num_buf);
	snprintf(num_buf, sizeof(num_buf), "%d", macro->times_encountered);
	xmlNewProp(ret, BAD_CAST "timesencountered", BAD_CAST num_buf);
	snprintf(num_buf, sizeof(num_buf), "%d", macro->times_called);
	xmlNewProp(ret, BAD_CAST "timescalled", BAD_CAST num_buf);
	snprintf(num_buf, sizeof(num_buf), "%d", macro->disabled_spin);
	xmlNewProp(ret, BAD_CAST "disabledspin", BAD_CAST num_buf);
	switch (macro->expansion_state)
	{
		case XPL_MACRO_EXPANDED: expansion_state = BAD_CAST "expanded"; break;
		case XPL_MACRO_EXPAND_ONCE: expansion_state = BAD_CAST "awaiting expansion"; break;
		case XPL_MACRO_EXPAND_ALWAYS: expansion_state = BAD_CAST "expand always"; break;
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
			expansion_state = BAD_CAST "unknown";
	}
	xmlNewProp(ret, BAD_CAST "expansionstate", expansion_state);
	return ret;
}

typedef struct _macroListScannerCtxt
{
	xmlHashTablePtr unique_hash;
	xmlNodePtr cur;
	xmlNodePtr head;
	xplQName tagname;
	xmlNodePtr parent;
} macroListScannerCtxt, *macroListScannerCtxtPtr;

static void _macroListScanner(void *payload, void *data, XML_HCBNC xmlChar *name)
{
	macroListScannerCtxtPtr ctxt = (macroListScannerCtxtPtr) data;
	xplMacroPtr macro = (xplMacroPtr) payload;
	xmlNodePtr cur;

	UNUSED_PARAM(name);
	if (ctxt->unique_hash && xplMacroGetFromHashByQName(ctxt->unique_hash, macro->qname))
		return;
	cur = xplMacroToNode((xplMacroPtr) payload, ctxt->tagname, ctxt->parent);
	if (ctxt->head)
	{
		ctxt->cur->next = cur;
		cur->prev = ctxt->cur;
	} else
		ctxt->head = cur;
	ctxt->cur = cur;
	if (ctxt->unique_hash)
		xplMacroAddToHash(ctxt->unique_hash, macro);
}

xmlNodePtr xplMacroTableToNodeList(const xmlNodePtr element, const xplQName tagname, bool unique, const xmlNodePtr parent)
{
	macroListScannerCtxt ctxt;
	xmlNodePtr cur;

	if (!element || !parent || !tagname.ncname)
		return NULL;

	ctxt.tagname = tagname;
	if (unique)
		ctxt.unique_hash = xmlHashCreate(cfgInitialMacroTableSize * 2);
	else
		ctxt.unique_hash = NULL;
	cur = element;
	ctxt.cur = ctxt.head = NULL;
	ctxt.parent = parent;
	while (cur && (cur->type == XML_ELEMENT_NODE))
	{
		if (cur->_private)
			xmlHashScan((xmlHashTablePtr) cur->_private, _macroListScanner, &ctxt);
		cur = cur->parent;
	}
	if (unique)
		xmlHashFree(ctxt.unique_hash, NULL);
	return ctxt.head;
}

typedef struct _xplMacroTableCopyUpScannerCtxt
{
	xmlNodePtr parent;
	xmlHashTablePtr table;
} xplMacroTableCopyUpScannerCtxt, *xplMacroTableCopyUpScannerCtxtPtr;

static void _macroTableCopyUpScanner(void *payload, void *data, XML_HCBNC xmlChar *name)
{
	xplMacroTableCopyUpScannerCtxtPtr ctxt = (xplMacroTableCopyUpScannerCtxtPtr) data;
	xplMacroPtr macro = (xplMacroPtr) payload;

	UNUSED_PARAM(name);
	if (!macro->disabled_spin && !xplMacroGetFromHashByQName(ctxt->table, macro->qname))
		xplMacroAddToHash(ctxt->table, xplMacroCopy(macro, ctxt->parent));
}

xmlHashTablePtr xplCloneMacroTableUpwards(const xmlNodePtr src, const xmlNodePtr parent)
{
	xplMacroTableCopyUpScannerCtxt ctxt;
	xmlNodePtr cur;

	ctxt.table = xmlHashCreate(cfgInitialMacroTableSize);
	ctxt.parent = parent;
	for (cur = src; cur; cur = cur->parent)
		xmlHashScan((xmlHashTablePtr) cur->_private, _macroTableCopyUpScanner, &ctxt);
	return ctxt.table;
}
