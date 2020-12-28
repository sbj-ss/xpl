#include <libxpl/xplcore.h>

void xplCmdStackPushEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplStackPushCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdStackPushEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdStackPushEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplPushToDocStack(commandInfo->document, commandInfo->element);
	ASSIGN_RESULT(NULL, false, true);
}
