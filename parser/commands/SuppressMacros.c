#include <libxml/chvalid.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmacro.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>
#include "commands/SuppressMacros.h"

void fillMacroHashFromNodeset(xmlNodePtr source, xmlHashTablePtr target, xmlNodeSetPtr nodeset)
{
	xplMacroPtr macro;
	xmlNodePtr cur;
	size_t i;

	for (i = 0; i < (size_t) nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		if (cur->type == XML_ELEMENT_NODE)
		{
			macro = xplMacroLookup(source, cur->ns? cur->ns->href: NULL, cur->name);
			if (macro)
				xmlHashAddEntry(target, cur->name, macro);
		} /* element */
	} /* for */
}

void fillMacroHashFromList(xmlNodePtr source, xmlHashTablePtr target, xmlChar *list)
{
	xmlChar *prev, *cur, *tagname;
	void *macro;
	xmlNsPtr ns;

	prev = cur = list;
	while (*cur)
	{
		if (*cur == ',')
		{
			*cur = 0;
			while(xmlIsBlank(*prev) && (prev < cur))
				prev++;
			EXTRACT_NS_AND_TAGNAME(prev, ns, tagname, source)
			macro = xplMacroLookup(source, ns? ns->href: NULL, tagname);
			if (macro)
				xmlHashAddEntry(target, prev, macro);
			prev = ++cur;
		} else
			cur += xstrGetOffsetToNextUTF8Char(cur);
	}
	if (*prev)
	{
		while(xmlIsBlank(*prev) && *prev)
			prev++;
		EXTRACT_NS_AND_TAGNAME(prev, ns, tagname, source)
		macro = xplMacroLookup(source, ns? ns->href: NULL, tagname);
		if (macro)
			xmlHashAddEntry(target, prev, macro);
	}
}

void switchMacro(void *payload, void *data, const xmlChar *name)
{
	((xplMacroPtr) payload)->disabled_spin += (int) (ptrdiff_t) data;
}

void xplCmdSuppressMacrosPrologue(xplCommandInfoPtr commandInfo)
{
#define SELECT_ATTR (BAD_CAST "select")
#define LIST_ATTR (BAD_CAST "list")

	xmlChar *select_attr = NULL;
	xmlChar *list_attr = NULL;
	xmlXPathObjectPtr sel = NULL;
	xmlHashTablePtr macros = NULL;

	select_attr = xmlGetNoNsProp(commandInfo->element, SELECT_ATTR);
	list_attr = xmlGetNoNsProp(commandInfo->element, LIST_ATTR);
	if (select_attr || list_attr)
	{
		if (select_attr)
		{
			sel = xplSelectNodes(commandInfo, commandInfo->element, select_attr);
			if (!sel)
			{
				xplDocDeferNodeListDeletion(commandInfo->document, xplDetachContent(commandInfo->element));
				xplSetChildren(commandInfo->element, xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid select XPath expression \"%s\"", select_attr));
				goto done;
			}
			if (sel->type != XPATH_NODESET)
			{
				xplDocDeferNodeListDeletion(commandInfo->document, xplDetachContent(commandInfo->element));
				xplSetChildren(commandInfo->element, xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath expression \"%s\" evaluated to non-nodeset value", select_attr));
				goto done;
			}
			if (sel->nodesetval)
			{
				macros = xmlHashCreate(16);
				fillMacroHashFromNodeset(commandInfo->element, macros, sel->nodesetval);
			}
		} 
		if (list_attr)
		{
			if (!macros)
				macros = xmlHashCreate(16);
			fillMacroHashFromList(commandInfo->element, macros, list_attr);
		}
		xmlHashScan(macros, switchMacro, (void*) 1);
	}
	commandInfo->prologue_state = macros;
done:
	if (sel) xmlXPathFreeObject(sel);
	if (select_attr) XPL_FREE(select_attr);
	if (list_attr) XPL_FREE(list_attr);
}

void xplCmdSuppressMacrosEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR BAD_CAST("repeat")
	xmlHashTablePtr macros;
	bool repeat;
	xmlNodePtr error;

	macros = (xmlHashTablePtr) commandInfo->prologue_state;
	if (macros)
	{
		xmlHashScan(macros, switchMacro, (void*) -1);
		xmlHashFree(macros, NULL);
	}
	if (commandInfo->element->type & XPL_NODE_DELETION_MASK)
		ASSIGN_RESULT(NULL, false, false);
	else {
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, false)))
			ASSIGN_RESULT(error, true, true);
		else
			ASSIGN_RESULT(xplDetachContent(commandInfo->element), repeat, true);
	}
}

xplCommand xplSuppressMacrosCommand = { 
	.prologue = xplCmdSuppressMacrosPrologue, 
	.epilogue = xplCmdSuppressMacrosEpilogue,
	.initializer = NULL,
	.finalizer = NULL,
	.flags = XPL_CMD_FLAG_CONTENT_SAFE
};
