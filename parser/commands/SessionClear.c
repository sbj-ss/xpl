#include <libxpl/xplcore.h>
#include <libxpl/xplsession.h>
#include <libxpl/xplutils.h>
#include "commands/SessionClear.h"

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
