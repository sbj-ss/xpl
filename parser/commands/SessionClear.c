#include "commands/SessionClear.h"
#include "Core.h"
#include "Session.h"
#include "Utils.h"

void xplCmdSessionClearPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdSessionClearEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (!commandInfo->document->main->session)
		return;
	xplSessionClear(commandInfo->document->main->session);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplSessionClearCommand = { xplCmdSessionClearPrologue, xplCmdSessionClearEpilogue };
