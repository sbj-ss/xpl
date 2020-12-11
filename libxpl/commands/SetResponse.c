#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdSetResponseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

void xplCmdSetResponseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlChar *response;
	if (!xplCheckNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "non-text nodes inside"), true, true);
		return;
	}
	response = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, true);
	if (commandInfo->document->response) XPL_FREE(commandInfo->document->response);
	commandInfo->document->response = response;
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplSetResponseCommand = { NULL, xplCmdSetResponseEpilogue };