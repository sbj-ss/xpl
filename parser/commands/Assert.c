#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplutils.h>
#include "commands/Assert.h"

void xplCmdAssertPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdAssertEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define MESSAGE_ATTR (BAD_CAST "message")
	xmlChar *txt = NULL;
	xmlChar *message_attr = NULL;
	xmlXPathObjectPtr ct = NULL;
	int smth = 0;

	if (!cfgEnableAssertions)
	{
		ASSIGN_RESULT(NULL, false, true);
		return;
	}
	if (!checkNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "condition is non-text"), true, true);
		return;
	}
	txt = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, true);
	if (!txt)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "condition is empty"), true, true);
		return;
	}
	message_attr = xmlGetNoNsProp(commandInfo->element, MESSAGE_ATTR);
	ct = xplSelectNodes(commandInfo->document, commandInfo->element, txt);
	if (ct)
	{
		switch(ct->type)
		{
		case XPATH_NODESET:
			smth = (ct->nodesetval)? ct->nodesetval->nodeNr: 0;
			break;
		case XPATH_BOOLEAN:
			smth = ct->boolval;
			break;
		case XPATH_NUMBER:
			smth = (ct->floatval != 0.0);
			break;
		case XPATH_STRING:
			smth = (ct->stringval && *ct->stringval);
			break;
		default:
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "unsupported XPath result type (expression is %s)", txt), true, true);
			goto done;
		}
		xmlXPathFreeObject(ct);
		if (!smth)
		{
			if (message_attr)
			{
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, message_attr), true, true);
			} else {
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "assertion \"%s\" failed", txt), true, true);
			}
		} else {
			ASSIGN_RESULT(NULL, false, true);
		}
	} else {
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid XPath expression (%s)", txt), true, true);
	}
done:
	if (txt) xmlFree(txt);
	if (message_attr) xmlFree(message_attr);
}

xplCommand xplAssertCommand = { xplCmdAssertPrologue, xplCmdAssertEpilogue };
