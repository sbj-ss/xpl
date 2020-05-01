#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include "commands/Namespace.h"

void xplCmdNamespacePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdNamespaceEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define PREFIX_ATTR (BAD_CAST "prefix")
#define DESTINATION_ATTR (BAD_CAST "destination")

	xmlChar *prefix_attr = NULL;
	xmlChar *destination_attr = NULL;
	xmlXPathObjectPtr sel = NULL;
	xmlChar *uri = NULL;
	xmlNodePtr ret = NULL;
	size_t i;

	prefix_attr = xmlGetNoNsProp(commandInfo->element, PREFIX_ATTR);
	destination_attr = xmlGetNoNsProp(commandInfo->element, DESTINATION_ATTR);
	if (!commandInfo->element->children)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "uri is empty"), true, true);
		goto done;
	}
	if (!checkNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "uri is non-text"), true, true);
		goto done;
	}
	uri = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, 1);
	if (!uri)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "uri is empty"), true, true);
		goto done;
	}
	if (destination_attr)
	{
		sel = xplSelectNodes(commandInfo->document, commandInfo->element, destination_attr);
		if (sel)
		{
			if (sel->type == XPATH_NODESET)
			{
				if (sel->nodesetval)
				{
					for (i = 0; i < (size_t) sel->nodesetval->nodeNr; i++)
					{
						if (sel->nodesetval->nodeTab[i]->type != XML_ELEMENT_NODE)
							continue;
						xmlNewNs(sel->nodesetval->nodeTab[i], uri, prefix_attr);
						/* ToDo: warnings */
					}
				}
			} else {
				ret = xplCreateErrorNode(commandInfo->element, BAD_CAST "destination XPath (%s) evaluated to non-nodeset value", destination_attr);
				goto done;
			}
		} else {
			ret = xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid destination XPath expression (%s)", destination_attr);
			goto done;
		}
	} else {
		xmlNewNs(commandInfo->element->parent, uri, prefix_attr);
		/* ToDo: warnings */
	}
done:
	if (sel)
		xmlXPathFreeObject(sel);
	if (prefix_attr)
		xmlFree(prefix_attr);
	if (destination_attr)
		xmlFree(destination_attr);
	if (uri)
		xmlFree(uri);
	ASSIGN_RESULT(ret, ret? true: false, true);
}

xplCommand xplNamespaceCommand = { xplCmdNamespacePrologue, xplCmdNamespaceEpilogue };
