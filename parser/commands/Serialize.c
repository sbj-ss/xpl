#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsave.h>
#include <libxpl/xpltree.h>
#include "commands/Serialize.h"

void xplCmdSerializePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdSerializeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define SELECT_ATTR (BAD_CAST "select")
	xmlChar *select_attr = NULL;
	xmlXPathObjectPtr obj = NULL;
	xmlChar *txt;
	xmlNodePtr ret;

	select_attr = xmlGetNoNsProp(commandInfo->element, SELECT_ATTR);
	if (select_attr)
	{
		obj = xplSelectNodes(commandInfo, commandInfo->element, select_attr);
		if (!obj)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid select XPath expression (%s)", select_attr), true, true);
			goto done;
		}
		if (obj->type != XPATH_NODESET) 
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath expression (%s) evaluated to non-nodeset value", select_attr), true, true);
			goto done;
		}
		txt = serializeNodeSet(obj->nodesetval);
	} else
		txt = serializeNodeList(commandInfo->element->children);
	ret = xmlNewDocText(commandInfo->element->doc, NULL);
	ret->content = txt;
	ASSIGN_RESULT(ret, false, true);
done:
	if (select_attr) XPL_FREE(select_attr);
	if (obj) xmlXPathFreeObject(obj);
}

xplCommand xplSerializeCommand = { xplCmdSerializePrologue, xplCmdSerializeEpilogue };
