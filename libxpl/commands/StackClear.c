#include <libxpl/xplcore.h>

void xplCmdStackClearEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplStackClearCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdStackClearEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdStackClearEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplClearDocStack(commandInfo->document);
	ASSIGN_RESULT(NULL, false, true);
}
