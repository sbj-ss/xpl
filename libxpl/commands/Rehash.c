#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>

void xplCmdRehashEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplRehashCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdRehashEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdRehashEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (!xplSessionGetSaMode(commandInfo->document->session))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), true, true);
		return;
	}
	xplLockThreads(true);
	xplReadConfig();
	xplLockThreads(false);
	ASSIGN_RESULT(NULL, false, true);
}
