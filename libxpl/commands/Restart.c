#include "libxpl/abstraction/xpr.h"
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsession.h>

void xplCmdRestartEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

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

xplCommand xplRestartCommand = { NULL, xplCmdRestartEpilogue };