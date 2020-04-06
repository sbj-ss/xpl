#include "commands/StackClear.h"
#include "Core.h"
#include "Utils.h"

void xplCmdStackClearPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdStackClearEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplClearDocStack(commandInfo->document);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplStackClearCommand = { xplCmdStackClearPrologue, xplCmdStackClearEpilogue };
