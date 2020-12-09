#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsession.h>
#include "commands/Shutdown.h"

void xplCmdShutdownEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (xplSessionGetSaMode(commandInfo->document->session))
		xprShutdownApp();
	else
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), true, true);
}

xplCommand xplShutdownCommand = { NULL, xplCmdShutdownEpilogue };
