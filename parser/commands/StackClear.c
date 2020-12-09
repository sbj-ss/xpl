#include <libxpl/xplcore.h>
#include "commands/StackClear.h"

void xplCmdStackClearEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplClearDocStack(commandInfo->document);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplStackClearCommand = { NULL, xplCmdStackClearEpilogue };
