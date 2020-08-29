#include <libxpl/xplcore.h>
#include <libxpl/xpltree.h>
#include "commands/Comment.h"

xplCommand xplCommentCommand =
{
	.prologue = xplCmdCommentPrologue,
	.epilogue = xplCmdCommentEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdCommentPrologue(xplCommandInfoPtr commandInfo)
{
	xplDocDeferNodeListDeletion(commandInfo->document, commandInfo->element->children);
	commandInfo->element->children = commandInfo->element->last = NULL;
}

void xplCmdCommentEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	UNUSED_PARAM(commandInfo);
	ASSIGN_RESULT(NULL, false, true);
}
