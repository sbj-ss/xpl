#include "commands/CommentNode.h"
#include "Messages.h"
#include "Utils.h"

void xplCmdCommentNodePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdCommentNodeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlChar *txt = NULL;
	xmlNodePtr ret;
	if (!checkNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "non-text nodes inside"), TRUE, TRUE);
		return;
	}
	txt = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, 1);
	ret = xmlNewDocComment(commandInfo->element->doc, NULL);
	ret->content = txt;
	ASSIGN_RESULT(ret, FALSE, TRUE);
}

xplCommand xplCommentNodeCommand = { xplCmdCommentNodePrologue, xplCmdCommentNodeEpilogue };
