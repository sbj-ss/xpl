#include <sys/time.h>
#include <libxpl/xplcore.h>

void xplCmdStartTimerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

void xplCmdStartTimerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	commandInfo->document->main->profile_checkpoint = time(NULL);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplStartTimerCommand = { NULL, xplCmdStartTimerEpilogue };
