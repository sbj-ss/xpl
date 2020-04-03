#include "commands/ParseXml.h"
#include "Messages.h"
#include "Utils.h"

void xplCmdParseXmlPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdParseXmlEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR (BAD_CAST "repeat")
	BOOL repeat;
	xmlChar *txt = NULL;
	xmlDocPtr doc = NULL;
	xmlChar *parse_error;
	xmlNodePtr ret, error;

	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, TRUE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		return;
	}
	if (!checkNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "non-text nodes inside"), TRUE, TRUE);
		return;
	}
	txt = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, 1);
	if (!txt)
	{
		ASSIGN_RESULT(NULL, FALSE, TRUE);
		return;
	}
	doc = xmlParseMemory((const char*) txt, xmlStrlen(txt));
	xmlFree(txt);
	if (!doc)
	{
		parse_error = getLastLibxmlError();
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "document parsing error: %s", parse_error), TRUE, TRUE);
		xmlFree(parse_error);
	} else {
		/* ToDo: replaceRedundantNamespaces */
		ret = detachContent((xmlNodePtr) doc);
		xmlSetTreeDoc(ret, commandInfo->element->doc);
		xmlFreeDoc(doc);
		ASSIGN_RESULT(ret, repeat, TRUE);
	}
}

xplCommand xplParseXmlCommand = { xplCmdParseXmlPrologue, xplCmdParseXmlEpilogue };
