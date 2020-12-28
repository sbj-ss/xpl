#include <sys/time.h>
#include <libxpl/xplcore.h>

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
	commandInfo->document->main->profile_checkpoint = time(NULL);
	ASSIGN_RESULT(NULL, false, true);
}
