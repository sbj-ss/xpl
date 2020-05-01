#include <libxpl/xplcore.h>
#include <libxpl/xpltree.h>
#include "commands/Comment.h"

void xplCmdCommentPrologue(xplCommandInfoPtr commandInfo)
{
	xplDocDeferNodeListDeletion(commandInfo->document, commandInfo->element->children);
	commandInfo->element->children = commandInfo->element->last = NULL;
}

void xplCmdCommentEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplCommentCommand = { xplCmdCommentPrologue, xplCmdCommentEpilogue };
