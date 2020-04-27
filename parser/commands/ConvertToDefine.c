#include <libxpl/xplcore.h>
#include <libxpl/xplmacro.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplutils.h>
#include "commands/ConvertToDefine.h"

void xplCmdConvertToDefinePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdConvertToDefineEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define DEFAULTEXPAND_ATTR (BAD_CAST "defaultexpand")
#define DEFAULTREPLACE_ATTR (BAD_CAST "defaultreplace")
	xmlChar *defaultexpand_attr = NULL;
	xmlNodePtr tmp;
	xplMacroExpansionState default_expand;
	bool default_replace = true;

	if ((defaultexpand_attr = xmlGetNoNsProp(commandInfo->element, DEFAULTEXPAND_ATTR)))
	{
		if ((default_expand = xplMacroExpansionStateFromString(defaultexpand_attr, true)) == XPL_MACRO_EXPAND_UNKNOWN)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "unknown defaultexpand value: \"%s\"", defaultexpand_attr), true, true);
			goto done;
		}
	} else
		default_expand = XPL_MACRO_EXPAND_NO_DEFAULT;
	if ((tmp = xplDecodeCmdBoolParam(commandInfo->element, DEFAULTREPLACE_ATTR, &default_replace, true)))
	{
		ASSIGN_RESULT(tmp, true, true);
		goto done;
	}
	tmp = commandInfo->element->children;
	while (tmp)
	{
		if (tmp->type == XML_ELEMENT_NODE)
			xplAddMacro(commandInfo->document, tmp, commandInfo->element->parent, true, default_expand, default_replace);
		tmp = tmp->next;
	}
	ASSIGN_RESULT(NULL, false, true);
done:
	if (defaultexpand_attr) xmlFree(defaultexpand_attr);
}

xplCommand xplConvertToDefineCommand = { xplCmdConvertToDefinePrologue, xplCmdConvertToDefineEpilogue };
