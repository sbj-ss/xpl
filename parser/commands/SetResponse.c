#include "commands/SetResponse.h"
#include "Core.h"
#include "Messages.h"
#include "Utils.h"

void xplCmdSetResponsePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdSetResponseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlChar *response;
	if (!checkNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "non-text nodes inside"), TRUE, TRUE);
		return;
	}
	response = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, TRUE);
	if (commandInfo->document->response) xmlFree(commandInfo->document->response);
	commandInfo->document->response = response;
	ASSIGN_RESULT(NULL, FALSE, TRUE);
}

xplCommand xplSetResponseCommand = { xplCmdSetResponsePrologue, xplCmdSetResponseEpilogue };
