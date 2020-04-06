#include "commands/StartTimer.h"
#include "Core.h"
#include <sys/time.h>

void xplCmdStartTimerPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdStartTimerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	commandInfo->document->main->profile_checkpoint = time(NULL);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplStartTimerCommand = { xplCmdStartTimerPrologue, xplCmdStartTimerEpilogue };
