#include "commands/Rename.h"
#include "Core.h"
#include "Messages.h"
#include "Utils.h"

void xplCmdRenamePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdRenameEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define SELECT_ATTR (BAD_CAST "select")
#define NEWNAME_ATTR (BAD_CAST "newname")

	xmlChar *select_attr = NULL;
	xmlChar *newname_attr = NULL;
	xmlXPathObjectPtr sel = NULL;
	xmlChar *new_name;
	xmlNsPtr ns = NULL;
	BOOL change_ns = FALSE;
	size_t i;
	xmlNodePtr cur;

	select_attr = xmlGetNoNsProp(commandInfo->element, SELECT_ATTR);
	if (!select_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select attribute is missing"), TRUE, TRUE);
		goto done;
	}
	newname_attr = xmlGetNoNsProp(commandInfo->element, NEWNAME_ATTR);
	if (!newname_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "newname attribute is missing"), TRUE, TRUE);
		goto done;
	}
	sel = xplSelectNodes(commandInfo->document, commandInfo->element, select_attr);
	if (!sel)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid select XPath: \"%s\"", select_attr), TRUE, TRUE);
		goto done;
	}
	if (sel->type != XPATH_NODESET)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath \"%s\" evaluated to non-nodeset value", select_attr), TRUE, TRUE);
		goto done;
	}
	{
		new_name = BAD_CAST xmlStrchr(newname_attr, ':');
		if (!new_name)
			new_name = newname_attr;
		else {
			*new_name++ = 0;
			ns = xmlSearchNs(commandInfo->element->doc, commandInfo->element, newname_attr);
			change_ns = TRUE;
		}
	}
	if (sel->nodesetval)
	{
		for (i = 0; i < (size_t) sel->nodesetval->nodeNr; i++)
		{
			cur = sel->nodesetval->nodeTab[i];
			if ((cur->type == XML_ELEMENT_NODE) || (cur->type == XML_ATTRIBUTE_NODE))
			{
				xmlNodeSetName(cur, new_name);
				if (change_ns)
					cur->ns = ns;
			}
		}
	}
	ASSIGN_RESULT(NULL, FALSE, TRUE);
done:
	if (select_attr)
		xmlFree(select_attr);
	if (newname_attr)
		xmlFree(newname_attr);
	if (sel)
		xmlXPathFreeObject(sel);
}

xplCommand xplRenameCommand = { xplCmdRenamePrologue, xplCmdRenameEpilogue };
