#include <libxpl/xplcore.h>
#include <libxpl/xplsession.h>

void xplCmdSessionClearEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplSessionClearCommand = {
	.prologue = NULL,
	.epilogue = xplCmdSessionClearEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdSessionClearEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (!commandInfo->document->main->session)
		return;
	xplSessionClear(commandInfo->document->main->session);
	ASSIGN_RESULT(NULL, false, true);
}
