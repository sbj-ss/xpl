#include <sys/time.h>
#include <libxpl/xplcore.h>
#include "commands/StartTimer.h"

void xplCmdStartTimerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	commandInfo->document->main->profile_checkpoint = time(NULL);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplStartTimerCommand = { NULL, xplCmdStartTimerEpilogue };
