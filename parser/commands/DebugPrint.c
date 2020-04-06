#include "commands/DebugPrint.h"
#include "Messages.h"
#include "Options.h"
#include "Utils.h"

void xplCmdDebugPrintPrologue(xplCommandInfoPtr commandInfo)
{

}

void xplCmdDebugPrintEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define SEVERITY_ATTR (BAD_CAST "severity")
	xmlChar *severity_attr = NULL;
	xmlChar *content = NULL;
	xplMsgType severity = xplMsgDebug;

	severity_attr = xmlGetNoNsProp(commandInfo->element, SEVERITY_ATTR);
	if (severity_attr)
		if ((severity = xplMsgTypeFromString(severity_attr, false)) == xplMsgUnknown)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "unknown severity value: \"%s\"", severity_attr), true, true);
			goto done;
		}
	if ((int) severity < cfgMinDebugPrintLevel) /* suppress unwanted noise */
	{
		ASSIGN_RESULT(NULL, false, true);
		goto done;
	}
	if (!checkNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "content is non-text"), true, true);
			goto done;
	} 
	if (commandInfo->element->children)
		content = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, 1);
	if (!content)
		content = xmlStrdup(BAD_CAST "<no text provided>");
	xplDisplayMessage(severity, BAD_CAST "%s", content);
	ASSIGN_RESULT(NULL, true, true);
done:
	if (content)
		xmlFree(content);
	if (severity_attr)
		xmlFree(severity_attr);
}

xplCommand xplDebugPrintCommand = { xplCmdDebugPrintPrologue, xplCmdDebugPrintEpilogue };
