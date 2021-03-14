#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcommand.h>
#include <libxpl/xplmessages.h>

void xplCmdDebugBreakEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplDebugBreakCommand = {
	.prologue = NULL,
	.epilogue = xplCmdDebugBreakEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdDebugBreakEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (xprIsDebuggerPresent())
		xprDebugBreak();
	else
		xplDisplayWarning(commandInfo->element, "called when running outside of IDE");
	ASSIGN_RESULT(NULL, false, true);
}
