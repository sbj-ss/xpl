#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdFatalEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

static xmlNodePtr _onlyFirstChildElement(xmlNodePtr parent)
{
	xmlNodePtr cur;

	if ((cur = xplFirstElementNode(parent->children)))
	{ 
		if (cur->next && cfgWarnOnMultipleSelection)
			xplDisplayWarning(parent, BAD_CAST "more nodes follow in command content, ignored");
		xplMakeNsSelfContainedTree(cur);
		xmlUnlinkNode(cur);
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
	root = xplFirstElementNode(commandInfo->element->doc->children);
	xplMarkAncestorAxisForDeletion(commandInfo->element->parent, root);
	ASSIGN_RESULT(NULL, false, false);
}
