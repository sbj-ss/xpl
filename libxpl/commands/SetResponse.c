#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdSetResponseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplSetResponseCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdSetResponseEpilogue,
	.flags = XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE,
	.params_stencil = NULL
};

void xplCmdSetResponseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (commandInfo->document->response)
		XPL_FREE(commandInfo->document->response);
	commandInfo->document->response = commandInfo->content;
	commandInfo->content = NULL;
	ASSIGN_RESULT(NULL, false, true);
}
