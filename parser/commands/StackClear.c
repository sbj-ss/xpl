#include <libxpl/xplcore.h>
#include <libxpl/xplutils.h>
#include "commands/StackClear.h"

void xplCmdStackClearPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdStackClearEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplClearDocStack(commandInfo->document);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplStackClearCommand = { xplCmdStackClearPrologue, xplCmdStackClearEpilogue };
