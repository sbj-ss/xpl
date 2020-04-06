#include "commands/Element.h"
#include "Messages.h"
#include "Utils.h"

void xplCmdElementPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdElementEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")
#define REPEAT_ATTR (BAD_CAST "repeat")

	xmlChar *name_attr = NULL;
	bool repeat;
	xmlNodePtr el, error;

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	if (!name_attr || !*name_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "name attribute is missing or empty"), true, true);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, true)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}

	el = createElement(commandInfo->element->parent, commandInfo->element, name_attr);
	if (!el)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "can't create element (possibly the name \"%s\" is invalid)", name_attr), true, true);
		goto done;
	}
	setChildren(el, detachContent(commandInfo->element));
	ASSIGN_RESULT(el, repeat, true);
done:
	if (name_attr) xmlFree(name_attr);
}

xplCommand xplElementCommand = { xplCmdElementPrologue, xplCmdElementEpilogue };
