#include <libxpl/xplcore.h>
#include <libxpl/xplsession.h>

void xplCmdSessionClearEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

void xplCmdSessionClearEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (!commandInfo->document->main->session)
		return;
	xplSessionClear(commandInfo->document->main->session);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplSessionClearCommand = { NULL, xplCmdSessionClearEpilogue };
