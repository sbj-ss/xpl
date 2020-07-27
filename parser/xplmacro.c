#include <stdio.h>
#include <string.h>
#include <libxpl/xplmacro.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

xplMacroExpansionState xplMacroExpansionStateFromString(xmlChar *state, bool allowNoDefault)
{
	if (!state)
		return XPL_MACRO_EXPAND_ALWAYS; /* default */
	if (!xmlStrcasecmp(state, BAD_CAST "true") || !xmlStrcasecmp(state, BAD_CAST "now"))
		return XPL_MACRO_EXPANDED;
	else if (!xmlStrcasecmp(state, BAD_CAST "false") || !xmlStrcasecmp(state, BAD_CAST "always"))
		return XPL_MACRO_EXPAND_ALWAYS;
	else if (!xmlStrcasecmp(state, BAD_CAST "once"))
		return XPL_MACRO_EXPAND_ONCE;
	else if (allowNoDefault && !xmlStrcasecmp(state, BAD_CAST "nodefault"))
		return XPL_MACRO_EXPAND_NO_DEFAULT;
	else
		return XPL_MACRO_EXPAND_UNKNOWN;
}

xplMacroPtr xplMacroCreate(xmlChar *aId, xmlNodePtr aContent, xplMacroExpansionState expansionState)
{
	xplMacroPtr macro = (xplMacroPtr) XPL_MALLOC(sizeof(xplMacro));
	if (!macro)
		return NULL;
	memset(macro, 0, sizeof(xplMacro));
	macro->id = XPL_STRDUP(aId);
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
	if (macro->name)
		XPL_FREE(macro->name);
	if (macro->ns && macro->ns_is_duplicated)
		xmlFreeNs(macro->ns);
	XPL_FREE(macro);
}

xplMacroPtr xplMacroCopy(xplMacroPtr macro, xmlNodePtr parent)
{
	xplMacroPtr ret;

	if (!macro)
		return NULL;
	ret = xplMacroCreate(macro->id, cloneNodeList(macro->content, parent, parent->doc), macro->expansion_state);
	if (!ret)
		return NULL;
	ret->name = XPL_STRDUP(macro->name);
	ret->ns_is_duplicated = true;
	ret->ns = xmlCopyNamespace(macro->ns);
	ret->line = -1;
	ret->parent = parent;
	return ret;
}

void xplMacroDeallocator(void *payload, xmlChar *name)
{
	xplMacroFree((xplMacroPtr) payload);
}

void xplMacroTableFree(xmlHashTablePtr macros)
{
	if (macros)
		xmlHashFree(macros, xplMacroDeallocator);
}

xplMacroPtr xplMacroLookup(xmlNodePtr element, const xmlChar *href, const xmlChar *name)
{
	xplMacroPtr macro = NULL;

	while (!macro && element && (element->type == XML_ELEMENT_NODE))
	{
		if (element->_private)
			/* Note name:ns-href, not ns-href:name */
			macro = (xplMacroPtr) xmlHashLookup2((xmlHashTablePtr) element->_private, name, href);
		element = element->parent;
	}
	return macro;
}

typedef struct
{
	xmlHashTablePtr unique_hash;
	int count;
	size_t len;
	xmlChar *cur;
	xmlChar *delimiter;
	size_t delimiter_len;
} macroStringScannerCtxt, *macroStringScannerCtxtPtr;

static void macroCountScanner(void *payload, void *data, xmlChar *name)
{
	macroStringScannerCtxtPtr ctxt = (macroStringScannerCtxtPtr) data;
	xplMacroPtr macro = (xplMacroPtr) payload;

	if (!ctxt->unique_hash || !xmlHashLookup2(ctxt->unique_hash, name, macro->ns? macro->ns->href: NULL))
	{
		ctxt->count++;
		ctxt->len += xmlStrlen(name);
		if (macro->ns)
		{
			ctxt->len++; /* ":" */
			ctxt->len += xmlStrlen(macro->ns->prefix);
		}
		if (ctxt->unique_hash)
			xmlHashAddEntry2(ctxt->unique_hash, name, macro->ns? macro->ns->href: NULL, (void*) 1);
	}
}

static void macroStringScanner(void *payload, void *data, xmlChar *name)
{
	macroStringScannerCtxtPtr ctxt = (macroStringScannerCtxtPtr) data;
	xplMacroPtr macro = (xplMacroPtr) payload;
	int len;

	if (!ctxt->unique_hash || !xmlHashLookup2(ctxt->unique_hash, name, macro->ns? macro->ns->href: NULL))
	{

		if (macro->ns)
		{
			len = xmlStrlen(macro->ns->prefix);
			memcpy(ctxt->cur, macro->ns->prefix, len);
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
			xmlHashAddEntry2(ctxt->unique_hash, name, macro->ns? macro->ns->href: NULL, (void*) 1);
	}
}

xmlChar* xplMacroTableToString(xmlNodePtr element, xmlChar* delimiter, bool unique)
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
			xmlHashScan((xmlHashTablePtr) cur->_private, macroCountScanner, &ctxt);
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
			xmlHashScan((xmlHashTablePtr) cur->_private, macroStringScanner, &ctxt);
		cur = cur->parent;
	}
	if (unique)
		xmlHashFree(ctxt.unique_hash, NULL);
	ctxt.cur -= ctxt.delimiter_len; /* skip last delimiter */
	*ctxt.cur = 0;
	return ret;
}

static xmlNodePtr xplMacroToNodeInner(xplMacroPtr macro, xmlNsPtr ns, xmlChar *tagName, xmlNodePtr parent)
{
	xmlNodePtr ret;
	xmlChar *expansion_state;
	char num_buf[12];
	xmlChar *parent_name = NULL;

	if (!macro || !parent)
		return NULL;
	ret = xmlNewDocNode(parent->doc, ns, tagName, NULL);
	xmlNewProp(ret, BAD_CAST "namespaceuri", macro->ns? macro->ns->href: NULL);
	xmlNewProp(ret, BAD_CAST "prefix", macro->ns? macro->ns->prefix: NULL);
	xmlNewProp(ret, BAD_CAST "name", macro->name);
	snprintf(num_buf, 12, "%d", macro->line);
	xmlNewProp(ret, BAD_CAST "line", BAD_CAST num_buf);
	if (macro->parent->ns && macro->parent->ns->href)
	{
		parent_name = XPL_STRDUP(macro->parent->ns->prefix);
		parent_name = xmlStrcat(parent_name, BAD_CAST ":");
	}
	parent_name = xmlStrcat(parent_name, macro->parent->name);
	xmlNewProp(ret, BAD_CAST "parentname", parent_name);
	XPL_FREE(parent_name);
	snprintf(num_buf, 12, "%d", macro->parent->line);
	xmlNewProp(ret, BAD_CAST "parentline", BAD_CAST num_buf);
	snprintf(num_buf, 12, "%d", macro->times_encountered);
	xmlNewProp(ret, BAD_CAST "timesencountered", BAD_CAST num_buf);
	snprintf(num_buf, 12, "%d", macro->times_called);
	xmlNewProp(ret, BAD_CAST "timescalled", BAD_CAST num_buf);
	snprintf(num_buf, 12, "%d", macro->disabled_spin);
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

xmlNodePtr xplMacroToNode(xplMacroPtr macro, xmlChar *tagName, xmlNodePtr parent)
{
	xmlNsPtr ns;
	xmlChar *tag_name;
	EXTRACT_NS_AND_TAGNAME(tagName, ns, tag_name, parent);
	return xplMacroToNodeInner(macro, ns, tag_name, parent);
}

typedef struct
{
	xmlHashTablePtr unique_hash;
	xmlNodePtr cur;
	xmlNodePtr head;
	xmlNsPtr ns;
	xmlChar *tag_name;
	xmlNodePtr parent;
} macroListScannerCtxt, *macroListScannerCtxtPtr;

static void macroListScanner(void *payload, void *data, xmlChar *name)
{
	macroListScannerCtxtPtr ctxt = (macroListScannerCtxtPtr) data;
	xplMacroPtr macro = (xplMacroPtr) payload;
	xmlNodePtr cur;

	if (!ctxt->unique_hash || !xmlHashLookup2(ctxt->unique_hash, name, macro->ns? macro->ns->href: NULL))
	{
		cur = xplMacroToNodeInner((xplMacroPtr) payload, ctxt->ns, ctxt->tag_name, ctxt->parent);
		if (ctxt->head)
		{
			ctxt->cur->next = cur;
			cur->prev = ctxt->cur;
		} else
			ctxt->head = cur;
		ctxt->cur = cur;
	}
	if (ctxt->unique_hash)
		xmlHashAddEntry2(ctxt->unique_hash, name, macro->ns? macro->ns->href: NULL, (void*) 1);
}

xmlNodePtr xplMacroTableToNodeList(xmlNodePtr element, xmlChar *tagQName, bool unique, xmlNodePtr parent)
{
	macroListScannerCtxt ctxt;
	xmlNodePtr cur;

	if (!element || !parent || !tagQName)
		return NULL;
	EXTRACT_NS_AND_TAGNAME(tagQName, ctxt.ns, ctxt.tag_name, parent);
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
			xmlHashScan((xmlHashTablePtr) cur->_private, macroListScanner, &ctxt);
		cur = cur->parent;
	}
	if (unique)
		xmlHashFree(ctxt.unique_hash, NULL);
	return ctxt.head;
}

typedef struct
{
	xmlNodePtr parent;
	xmlHashTablePtr table;
} xplMacroTableCopyUpScannerCtxt, *xplMacroTableCopyUpScannerCtxtPtr;

static void macroTableCopyUpScanner(void *payload, void *data, xmlChar *name)
{
	xplMacroTableCopyUpScannerCtxtPtr ctxt = (xplMacroTableCopyUpScannerCtxtPtr) data;
	xplMacroPtr macro = (xplMacroPtr) payload;

	if (!macro->disabled_spin && !xmlHashLookup2(ctxt->table, macro->name, macro->ns? macro->ns->href: NULL))
		xmlHashAddEntry2(ctxt->table, name, macro->ns? macro->ns->href: NULL, xplMacroCopy(macro, ctxt->parent));
}

xmlHashTablePtr xplCloneMacroTableUpwards(xmlNodePtr src, xmlNodePtr parent)
{
	xplMacroTableCopyUpScannerCtxt ctxt;
	ctxt.table = xmlHashCreate(cfgInitialMacroTableSize);
	ctxt.parent = parent;
	while (src)
	{
		xmlHashScan((xmlHashTablePtr) src->_private, macroTableCopyUpScanner, &ctxt);
		src = src->parent;
	}
	return ctxt.table;
}
