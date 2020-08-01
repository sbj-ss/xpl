#include <libxpl/xplcore.h>
#include <libxpl/xplmacro.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include "commands/ReplaceIfUndefined.h"

void xplCmdReplaceIfUndefinedPrologue(xplCommandInfoPtr commandInfo)
{
#define NAME_ATTR (BAD_CAST "name")

	xmlChar *name_attr = NULL;
	xmlChar *tagname;
	xmlNsPtr ns = NULL;

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	if (!name_attr)
	{
		commandInfo->prologue_error = xplCreateErrorNode(commandInfo->element, BAD_CAST "missing name attribute");
		xplDocDeferNodeListDeletion(commandInfo->document, xplDetachContent(commandInfo->element));
		return;
	}
	EXTRACT_NS_AND_TAGNAME(name_attr, ns, tagname, commandInfo->element);
	if (xplMacroLookup(commandInfo->element->parent, ns? ns->href: NULL, tagname))
	{
		xplDocDeferNodeListDeletion(commandInfo->document, xplDetachContent(commandInfo->element));
		commandInfo->prologue_error = xmlNewDocNode(commandInfo->element->doc, ns, tagname, NULL);
	}
	XPL_FREE(name_attr);
}

void xplCmdReplaceIfUndefinedEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (commandInfo->prologue_error)
		ASSIGN_RESULT(commandInfo->prologue_error, true, true);
	else
		ASSIGN_RESULT(xplDetachContent(commandInfo->element), false, true);
}

xplCommand xplReplaceIfUndefinedCommand = { 
	.prologue = xplCmdReplaceIfUndefinedPrologue, 
	.epilogue = xplCmdReplaceIfUndefinedEpilogue,
	.initializer = NULL,
	.finalizer = NULL,
	.flags = 0
};

