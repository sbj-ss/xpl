#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplmessages.h>
#include "commands/DebugBreak.h"

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
		xplDisplayMessage(xplMsgWarning, BAD_CAST "xpl:debug-break called when running outside of IDE (file \"%s\", line %d)", commandInfo->element->doc->URL, commandInfo->element->line);
	ASSIGN_RESULT(NULL, false, true);
}
