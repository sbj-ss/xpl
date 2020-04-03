#include "commands/Shutdown.h"
#include "Core.h"
#include "Messages.h"
#include "Session.h"
#include "Utils.h"

void xplCmdShutdownPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdShutdownEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (xplSessionGetSaMode(commandInfo->document->session))
		xprShutdownApp();
	else
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), TRUE, TRUE);
}

xplCommand xplShutdownCommand = { xplCmdShutdownPrologue, xplCmdShutdownEpilogue };
