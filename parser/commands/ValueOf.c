#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplutils.h>
#include "commands/ValueOf.h"

void xplCmdValueOfPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdValueOfEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define SELECT_ATTR (BAD_CAST "select")
	xmlXPathObjectPtr sel = NULL;
	xmlChar *select_attr;
	xmlChar *value;
	xmlNodePtr ret;
	
	select_attr = xmlGetNoNsProp(commandInfo->element, SELECT_ATTR);
	if (!select_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "missing select attribute"), true, true);
		return;
	}
	 sel = xplSelectNodes(commandInfo->document, commandInfo->element, select_attr);
	if (sel)
	{
		if ((sel->type == XPATH_BOOLEAN) || (sel->type == XPATH_NUMBER) || (sel->type == XPATH_STRING))
		{
			value = xmlXPathCastToString(sel);
			ret = xmlNewDocText(commandInfo->document->document, NULL);
			ret->content = value;
			ASSIGN_RESULT(ret, false, true);
		} else {
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath expression \"%s\" evaluated to non-scalar value", select_attr), true, true);
		}
		xmlXPathFreeObject(sel);
	} else {
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid select XPath expression \"%s\"", select_attr), true, true);
	}
	xmlFree(select_attr);
}

xplCommand xplValueOfCommand = { xplCmdValueOfPrologue, xplCmdValueOfEpilogue };
