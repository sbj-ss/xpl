#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>
#include "commands/Test.h"

void xplCmdTestPrologue(xplCommandInfoPtr commandInfo)
{
}

static xmlNodePtr createBreak(xplCommandInfoPtr commandInfo, xmlNodePtr error, xmlChar *point)
{
	xmlNsPtr xpl_ns;
	xmlNodePtr ret;

	if (!(xpl_ns = commandInfo->document->root_xpl_ns))
		xpl_ns = xmlSearchNsByHref(commandInfo->element->doc, commandInfo->element, cfgXplNsUri);
	ret = xmlNewDocNode(commandInfo->element->doc, xpl_ns, BAD_CAST "break", NULL);
	if (point)
		xmlNewProp(ret, BAD_CAST "point", point);
	if (error)
	{
		xmlAddNextSibling(error, ret);
		ret = error;
	}
	return ret;
}

void xplCmdTestEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR (BAD_CAST "repeat")
#define POINT_ATTR (BAD_CAST "point")
	xmlChar *point_attr = NULL;
	xmlChar *txt = NULL;
	xmlXPathObjectPtr ct = NULL;
	int smth = 0;
	xmlNodePtr parent, error;
	bool repeat = true;

	ASSIGN_RESULT(NULL, false, true);
	if (!xplCheckNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(createBreak(commandInfo, xplCreateErrorNode(commandInfo->element, BAD_CAST "condition is non-text"), NULL), true, true);
		return;
	}
	txt = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, true);
	if (!txt)
	{
		ASSIGN_RESULT(createBreak(commandInfo, xplCreateErrorNode(commandInfo->element, BAD_CAST "condition is empty"), NULL), true, true);
		return;
	}
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
			ASSIGN_RESULT(createBreak(commandInfo, xplCreateErrorNode(commandInfo->element, BAD_CAST "unsupported XPath result type (expression is %s)", txt), NULL), true, true);
			goto done;
		}
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, true)))
		{
			ASSIGN_RESULT(createBreak(commandInfo, error, NULL), true, true);
			goto done;
		}
		if (!smth)
		{
			point_attr = xmlGetNoNsProp(commandInfo->element, POINT_ATTR);
			ASSIGN_RESULT(createBreak(commandInfo, NULL, point_attr), repeat, true);
			if (point_attr) XPL_FREE(point_attr);
		} else {
			parent = commandInfo->element->parent;
			if (parent && parent->ns && !xmlStrcmp(parent->name, BAD_CAST("when")) && xplCheckNodeForXplNs(commandInfo->document, parent))
				xmlAddNextSibling(parent, createBreak(commandInfo, NULL, NULL));
		}
		xmlXPathFreeObject(ct);
	} else {
		 ASSIGN_RESULT(createBreak(commandInfo, xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid XPath expression (%s)", txt), NULL), true, true);
	}
done:
	if (txt) XPL_FREE(txt);
}

xplCommand xplTestCommand = { xplCmdTestPrologue, xplCmdTestEpilogue };

