#include <libxpl/xplcommand.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdCommentNodeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);


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
