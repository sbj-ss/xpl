#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include "commands/CommentNode.h"

xplCommand xplCommentNodeCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdCommentNodeEpilogue,
	.flags = XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE,
	.params_stencil = NULL
};

void xplCmdCommentNodeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	ASSIGN_RESULT(xmlNewDocComment(commandInfo->element->doc, commandInfo->content), false, true);
}
