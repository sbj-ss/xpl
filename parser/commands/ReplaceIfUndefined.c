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
		commandInfo->_private = xplCreateErrorNode(commandInfo->element, BAD_CAST "missing name attribute");
		xplDocDeferNodeListDeletion(commandInfo->document, detachContent(commandInfo->element));
		return;
	}
	EXTRACT_NS_AND_TAGNAME(name_attr, ns, tagname, commandInfo->element);
	if (xplMacroLookup(commandInfo->element->parent, ns? ns->href: NULL, tagname))
	{
		xplDocDeferNodeListDeletion(commandInfo->document, detachContent(commandInfo->element));
		commandInfo->_private = xmlNewDocNode(commandInfo->element->doc, ns, tagname, NULL);
	}
	xmlFree(name_attr);
}

void xplCmdReplaceIfUndefinedEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (commandInfo->_private)
		ASSIGN_RESULT((xmlNodePtr) commandInfo->_private, true, true);
	else
		ASSIGN_RESULT(detachContent(commandInfo->element), false, true);
}

xplCommand xplReplaceIfUndefinedCommand = { 
	SFINIT(.prologue, xplCmdReplaceIfUndefinedPrologue), 
	SFINIT(.epilogue, xplCmdReplaceIfUndefinedEpilogue),
	SFINIT(.initializer, NULL),
	SFINIT(.finalizer, NULL),
	SFINIT(.flags, 0)
};

