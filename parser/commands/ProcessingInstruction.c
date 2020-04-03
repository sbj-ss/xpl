#include "commands/ProcessingInstruction.h"
#include "Messages.h"
#include "Utils.h"

void xplCmdProcessingInstructionPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdProcessingInstructionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")
	xmlChar *name_attr = NULL;
	xmlChar *txt;
	xmlNodePtr ret;

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	if (!name_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "missing name attribute"), TRUE, TRUE);
		goto done;
	}
	if (!checkNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "non-text nodes inside"), TRUE, TRUE);
		goto done;
	}
	txt = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, 1);
	if (!txt)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "missing content"), TRUE, TRUE);
		goto done;
	}
	ret = xmlNewDocPI(commandInfo->element->doc, name_attr, NULL);
	ret->content = txt;
	ASSIGN_RESULT(ret, FALSE, TRUE);
done:
	if (name_attr)
		xmlFree(name_attr);

}

xplCommand xplProcessingInstructionCommand = { xplCmdProcessingInstructionPrologue, xplCmdProcessingInstructionEpilogue };
