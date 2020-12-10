#include <libxpl/xplcommand.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdProcessingInstructionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

void xplCmdProcessingInstructionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")
	xmlChar *name_attr = NULL;
	xmlChar *txt;
	xmlNodePtr ret;

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	if (!name_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "missing name attribute"), true, true);
		goto done;
	}
	if (!xplCheckNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "non-text nodes inside"), true, true);
		goto done;
	}
	txt = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, 1);
	if (!txt)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "missing content"), true, true);
		goto done;
	}
	ret = xmlNewDocPI(commandInfo->element->doc, name_attr, NULL);
	ret->content = txt;
	ASSIGN_RESULT(ret, false, true);
done:
	if (name_attr)
		XPL_FREE(name_attr);

}

xplCommand xplProcessingInstructionCommand = { NULL, xplCmdProcessingInstructionEpilogue };
