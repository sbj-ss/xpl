#include <libxpl/xplcore.h>
#include <libxpl/abstraction/xpr.h>

void xplCmdStartTimerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplStartTimerCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdStartTimerEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdStartTimerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xprGetTime(&commandInfo->document->main->profile_start_time);
	ASSIGN_RESULT(NULL, false, true);
}
