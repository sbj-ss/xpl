#include <libxpl/xplcore.h>
#include <libxpl/xpltree.h>
#include "commands/Fatal.h"

static xmlNodePtr _onlyFirstChildElement(xmlNodePtr parent)
{
	xmlNodePtr cur, tmp;

	cur = xplDetachContent(parent);
	/* remove all non-elements */
	while (cur && cur->type != XML_ELEMENT_NODE)
	{
		tmp = cur->next;
		xmlUnlinkNode(cur);
		xmlFreeNode(cur);
		cur = tmp;
	}
	if (cur) /* element found */
	{ 
		/* delete tail */
		tmp = cur->next;
		if (tmp)
		{
			cur->next = NULL;
			tmp->prev = NULL;
			xmlFreeNodeList(tmp);
		}
		return cur;
	}
	/* garbage inside, use predefined message */
	return xmlNewDocNode(parent->doc, NULL, BAD_CAST "error", BAD_CAST "fatal command called");
}

xplCommand xplFatalCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdFatalEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdFatalEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlNodePtr root;

	commandInfo->document->fatal_content = _onlyFirstChildElement(commandInfo->element);
	root = commandInfo->element->doc->children;
	while (root && root->type != XML_ELEMENT_NODE)
		root = root->next;
	xplMarkAncestorAxisForDeletion(commandInfo->element->parent, root);
	ASSIGN_RESULT(NULL, false, true);
}
