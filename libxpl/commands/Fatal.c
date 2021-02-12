#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdFatalEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

static xmlNodePtr _onlyFirstChildElement(xmlNodePtr parent)
{
	xmlNodePtr cur = parent->children;

	/* skip all non-elements */
	while (cur && cur->type != XML_ELEMENT_NODE)
		cur = cur->next;
	if (cur) /* element found */
	{ 
		if (cur->next)
		{
			if (cfgWarnOnMultipleSelection)
				xplDisplayWarning(parent, BAD_CAST "more nodes follow in command content, deleted");
			if (cur->prev)
			{
				cur->prev->next = cur->next;
				cur->next->prev = cur->prev;
			} else {
				cur->next->prev = NULL;
				cur->parent->children = cur->next;
			}
		} else if (!(cur->parent->last = cur->prev))
			cur->parent->children = NULL;
		cur->next = cur->prev = NULL;
		xplMakeNsSelfContainedTree(cur);
		cur->parent = NULL;
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
	ASSIGN_RESULT(NULL, false, false);
}
