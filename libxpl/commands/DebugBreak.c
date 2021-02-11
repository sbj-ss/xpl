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
		xplDisplayMessage(XPL_MSG_WARNING, BAD_CAST "xpl:debug-break called when running outside of IDE (file \"%s\", line %d)", commandInfo->element->doc->URL, commandInfo->element->line);
	ASSIGN_RESULT(NULL, false, true);
}
