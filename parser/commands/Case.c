#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>
#include "commands/Case.h"

xplCommand xplCaseCommand = { xplCmdCasePrologue, xplCmdCaseEpilogue };

void xplCmdCasePrologue(xplCommandInfoPtr commandInfo)
{
#define KEY_ATTR (BAD_CAST "key")
#define BREAK_ATTR (BAD_CAST "break")
	xmlChar *key_attr = NULL;
	bool do_break;
	xmlXPathObjectPtr sel = NULL, parent_sel;
	xmlNodePtr parent = commandInfo->element->parent;
	xmlNodePtr error;

	if (!parent->ns || 
		((parent->ns != commandInfo->document->root_xpl_ns) && xmlStrcmp(parent->ns->href, cfgXplNsUri)) ||
		xmlStrcmp(parent->name, BAD_CAST "switch"))
	{
		commandInfo->prologue_error = xplCreateErrorNode(commandInfo->element, BAD_CAST "parent element must be a switch command");
		goto done;
	}
	key_attr = xmlGetNoNsProp(commandInfo->element, KEY_ATTR);
	if (!key_attr)
	{
		commandInfo->prologue_error = xplCreateErrorNode(commandInfo->element, BAD_CAST "missing key attribute");
		goto done;
	}
	sel = xplSelectNodes(commandInfo, commandInfo->element, key_attr);
	if (!sel)
	{
		commandInfo->prologue_error = xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid key XPath expression \"%s\"", key_attr);
		goto done;
	}
	parent_sel = (xmlXPathObjectPtr) parent->content;
	/* ToDo: param for equality/identity */
	if (xplCompareXPathSelections(sel, parent_sel, 0))
	{
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, BREAK_ATTR, &do_break, true)))
		{
			commandInfo->prologue_error = error;
			goto done;
		}
		if (do_break)
		{
			xplDocDeferNodeListDeletion(commandInfo->document, commandInfo->element->next);
			commandInfo->element->next = NULL;
			parent->last = commandInfo->element;
		}
	} else
		xplDocDeferNodeListDeletion(commandInfo->document, xplDetachContent(commandInfo->element));
	if (sel->nodesetval)
		sel->nodesetval->nodeNr = 0;
done:
	if (commandInfo->prologue_error)
		xplDocDeferNodeListDeletion(commandInfo->document, xplDetachContent(commandInfo->element));
	if (key_attr) XPL_FREE(key_attr);
	if (sel)
		xmlXPathFreeObject(sel);
}

void xplCmdCaseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR (BAD_CAST "repeat")
	xmlChar *repeat_attr = NULL;
	bool repeat = false;

	if (commandInfo->prologue_error)
	{
		ASSIGN_RESULT(commandInfo->prologue_error, true, true);
	} else {
		if ((repeat_attr = xmlGetNoNsProp(commandInfo->element, REPEAT_ATTR)))
		{
			if (!xmlStrcasecmp(repeat_attr, BAD_CAST "true"))
				repeat = true;
			XPL_FREE(repeat_attr);
		}
		ASSIGN_RESULT(xplDetachContent(commandInfo->element), repeat, true);
	}
}
