#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>
#include "commands/ParseXml.h"

void xplCmdParseXmlPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdParseXmlEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR (BAD_CAST "repeat")
	bool repeat;
	xmlChar *txt = NULL;
	xmlDocPtr doc = NULL;
	xmlChar *parse_error;
	xmlNodePtr ret, error;

	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, true)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	if (!checkNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "non-text nodes inside"), true, true);
		return;
	}
	txt = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, 1);
	if (!txt)
	{
		ASSIGN_RESULT(NULL, false, true);
		return;
	}
	doc = xmlParseMemory((const char*) txt, xmlStrlen(txt));
	XPL_FREE(txt);
	if (!doc)
	{
		parse_error = getLastLibxmlError();
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "document parsing error: %s", parse_error), true, true);
		XPL_FREE(parse_error);
	} else {
		/* ToDo: replaceRedundantNamespaces */
		ret = detachContent((xmlNodePtr) doc);
		xmlSetTreeDoc(ret, commandInfo->element->doc);
		xmlFreeDoc(doc);
		ASSIGN_RESULT(ret, repeat, true);
	}
}

xplCommand xplParseXmlCommand = { xplCmdParseXmlPrologue, xplCmdParseXmlEpilogue };
