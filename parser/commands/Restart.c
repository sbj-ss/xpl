#include "libxpl/abstraction/xpr.h"
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsession.h>
#include "commands/Restart.h"

void xplCmdRestartPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdRestartEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (!xplSessionGetSaMode(commandInfo->document->session))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), true, true);
		return;
	}
	xplLockThreads(true);
	xprSpawnProcessCopy();
	xprShutdownApp();
}

xplCommand xplRestartCommand = { xplCmdRestartPrologue, xplCmdRestartEpilogue };
