#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplmessages.h>
#include "commands/DebugBreak.h"

void xplCmdDebugBreakPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdDebugBreakEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (xprIsDebuggerPresent())
		xprDebugBreak();
	else
		xplDisplayMessage(xplMsgWarning, BAD_CAST "xpl:debug-break called when running outside of IDE (file \"%s\", line %d)", commandInfo->element->doc->URL, commandInfo->element->line);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplDebugBreakCommand = { xplCmdDebugBreakPrologue, xplCmdDebugBreakEpilogue };

