#include "commands/DebugBreak.h"
#include "Messages.h"
#include "abstraction/xpr.h"

void xplCmdDebugBreakPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdDebugBreakEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (xprIsDebuggerPresent())
		xprDebugBreak();
	else
		xplDisplayMessage(xplMsgWarning, BAD_CAST "xpl:debug-break called when running outside of IDE (file \"%s\", line %d)", commandInfo->element->doc->URL, commandInfo->element->line);
	ASSIGN_RESULT(NULL, FALSE, TRUE);
}

xplCommand xplDebugBreakCommand = { xplCmdDebugBreakPrologue, xplCmdDebugBreakEpilogue };

