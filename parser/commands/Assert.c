#include "commands/Assert.h"
#include "Core.h"
#include "Messages.h"
#include "Options.h"
#include "Utils.h"

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
		ASSIGN_RESULT(NULL, FALSE, TRUE);
		return;
	}
	if (!checkNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "condition is non-text"), TRUE, TRUE);
		return;
	}
	txt = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, TRUE);
	if (!txt)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "condition is empty"), TRUE, TRUE);
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
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "unsupported XPath result type (expression is %s)", txt), TRUE, TRUE);
			goto done;
		}
		xmlXPathFreeObject(ct);
		if (!smth)
		{
			if (message_attr)
			{
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, message_attr), TRUE, TRUE);
			} else {
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "assertion \"%s\" failed", txt), TRUE, TRUE);
			}
		} else {
			ASSIGN_RESULT(NULL, FALSE, TRUE);
		}
	} else {
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid XPath expression (%s)", txt), TRUE, TRUE);
	}
done:
	if (txt) xmlFree(txt);
	if (message_attr) xmlFree(message_attr);
}

xplCommand xplAssertCommand = { xplCmdAssertPrologue, xplCmdAssertEpilogue };
