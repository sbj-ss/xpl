#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsession.h>
#include <libxpl/xplutils.h>
#include "commands/Restart.h"
#include "abstraction/xpr.h"

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
	xplLockThreads(TRUE);
	xprSpawnProcessCopy();
	xprShutdownApp();
}

xplCommand xplRestartCommand = { xplCmdRestartPrologue, xplCmdRestartEpilogue };
