#include "commands/Fatal.h"
#include "Core.h"
#include "Utils.h"

void xplCmdFatalPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdFatalEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlNodePtr ct, cur, root_element;

	cur = ct = detachContent(commandInfo->element);
	/* remove all non-elements */
	while (ct && ct->type != XML_ELEMENT_NODE)
	{
		ct = cur->next;
		xmlUnlinkNode(cur);
		xmlFreeNode(cur);
		cur = ct;
	}
	if (ct) /* element found */
	{ 
		/* delete tail */
		cur = ct->next;
		if (cur)
		{
			ct->next = NULL;
			cur->prev = NULL;
			xmlFreeNodeList(cur);
		}
	} else { /* garbage inside, use predefined message */
		ct = xmlNewDocNode(commandInfo->element->doc, NULL, BAD_CAST "error", BAD_CAST "fatal command called");
	}
	commandInfo->document->fatal_content = ct;
	root_element = commandInfo->element->doc->children;
	while (root_element)
	{
		if (root_element->type == XML_ELEMENT_NODE)
			break;
		root_element = root_element->next;
	}
	markAncestorAxisForDeletion(commandInfo->element->parent, root_element);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplFatalCommand = { xplCmdFatalPrologue, xplCmdFatalEpilogue };

