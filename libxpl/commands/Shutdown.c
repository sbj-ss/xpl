#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsession.h>

void xplCmdShutdownEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplShutdownCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdShutdownEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdShutdownEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (xplSessionGetSaMode(commandInfo->document->session))
		xprShutdownApp();
	else
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), true, true);
}
