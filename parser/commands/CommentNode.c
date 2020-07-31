#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include "commands/CommentNode.h"

void xplCmdCommentNodePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdCommentNodeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlChar *txt = NULL;
	xmlNodePtr ret;
	if (!xplCheckNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "non-text nodes inside"), true, true);
		return;
	}
	txt = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, 1);
	ret = xmlNewDocComment(commandInfo->element->doc, NULL);
	ret->content = txt;
	ASSIGN_RESULT(ret, false, true);
}

xplCommand xplCommentNodeCommand = { xplCmdCommentNodePrologue, xplCmdCommentNodeEpilogue };
