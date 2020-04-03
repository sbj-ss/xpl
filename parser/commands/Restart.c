#include "commands/Restart.h"
#include "Core.h"
#include "Messages.h"
#include "Session.h"
#include "Utils.h"
#include "abstraction/xpr.h"

void xplCmdRestartPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdRestartEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (!xplSessionGetSaMode(commandInfo->document->session))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), TRUE, TRUE);
		return;
	}
	xplLockThreads(TRUE);
	xprSpawnProcessCopy();
	xprShutdownApp();
}

xplCommand xplRestartCommand = { xplCmdRestartPrologue, xplCmdRestartEpilogue };
