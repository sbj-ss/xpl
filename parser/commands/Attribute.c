#include "commands/Attribute.h"
#include "Core.h"
#include "Messages.h"
#include "Utils.h"

void xplCmdAttributePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdAttributeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")
#define DST_ATTR (BAD_CAST "destination")
#define REPLACE_ATTR (BAD_CAST "replace")
#define FORCEBLANK_ATTR (BAD_CAST "forceblank")

	xmlChar *txt = NULL;
	xmlChar *name_attr = NULL;
	xmlChar *dst_attr = NULL;
	BOOL forceblank;
	BOOL replace;
	xmlXPathObjectPtr dest_list = NULL;
	xmlChar *attr_value;
	xmlNodePtr cur, error;
	size_t i;
	
	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	if (!name_attr || !*name_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "name attribute is missing or empty"), TRUE, TRUE);
		return;
	}

	if (!checkNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "non-text nodes in content"), TRUE, TRUE);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, FORCEBLANK_ATTR, &forceblank, FALSE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		goto done;
	}
	txt = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, TRUE);
	if (!txt)
	{
		if (!forceblank)
		{
			ASSIGN_RESULT(NULL, FALSE, TRUE);
			goto done; // empty value, nothing to do
		} else
			attr_value = BAD_CAST "";
	} else
		attr_value = txt;
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPLACE_ATTR, &replace, TRUE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		goto done;
	}

	dst_attr = xmlGetNoNsProp(commandInfo->element, DST_ATTR);
	if (dst_attr)
	{
		dest_list = xplSelectNodes(commandInfo->document, commandInfo->element, dst_attr);
		if (dest_list)
		{
			if (dest_list->type == XPATH_NODESET)
			{
				if (dest_list->nodesetval)
				{
					for (i = 0; i < (size_t) dest_list->nodesetval->nodeNr; i++)
					{
						cur = dest_list->nodesetval->nodeTab[i];
						if (cur->type == XML_ELEMENT_NODE)
							assignAttribute(commandInfo->element, cur, name_attr, attr_value, replace);
					}
				}
			} else {
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "destination XPath (%s) evaluated to scalar or undefined", dst_attr), TRUE, TRUE);
				goto done;
			}
		} else {
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid destination XPath (%s)", dst_attr), TRUE, TRUE);
			xmlResetLastError();
			goto done;
		}
	} else if (commandInfo->element->parent) 
		assignAttribute(commandInfo->element, commandInfo->element->parent, name_attr, attr_value, replace);
	ASSIGN_RESULT(NULL, FALSE, TRUE);
done:
	if (txt) xmlFree(txt);
	if (name_attr) xmlFree(name_attr);
	if (dst_attr) xmlFree(dst_attr);
	if (dest_list) xmlXPathFreeObject(dest_list);
}

xplCommand xplAttributeCommand = { xplCmdAttributePrologue, xplCmdAttributeEpilogue };
