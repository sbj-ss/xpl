#include "commands/ConvertToDefine.h"
#include "Core.h"
#include "Macro.h"
#include "Messages.h"
#include "Utils.h"

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
	BOOL default_replace = TRUE;

	if ((defaultexpand_attr = xmlGetNoNsProp(commandInfo->element, DEFAULTEXPAND_ATTR)))
	{
		if ((default_expand = xplMacroExpansionStateFromString(defaultexpand_attr, TRUE)) == XPL_MACRO_EXPAND_UNKNOWN)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "unknown defaultexpand value: \"%s\"", defaultexpand_attr), TRUE, TRUE);
			goto done;
		}
	} else
		default_expand = XPL_MACRO_EXPAND_NO_DEFAULT;
	if ((tmp = xplDecodeCmdBoolParam(commandInfo->element, DEFAULTREPLACE_ATTR, &default_replace, TRUE)))
	{
		ASSIGN_RESULT(tmp, TRUE, TRUE);
		goto done;
	}
	tmp = commandInfo->element->children;
	while (tmp)
	{
		if (tmp->type == XML_ELEMENT_NODE)
			xplAddMacro(commandInfo->document, tmp, commandInfo->element->parent, TRUE, default_expand, default_replace);
		tmp = tmp->next;
	}
	ASSIGN_RESULT(NULL, FALSE, TRUE);
done:
	if (defaultexpand_attr) xmlFree(defaultexpand_attr);
}

xplCommand xplConvertToDefineCommand = { xplCmdConvertToDefinePrologue, xplCmdConvertToDefineEpilogue };
