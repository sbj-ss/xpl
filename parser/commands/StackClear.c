#include <libxpl/xplcore.h>

void xplCmdStackClearEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

void xplCmdStackClearEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplClearDocStack(commandInfo->document);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplStackClearCommand = { NULL, xplCmdStackClearEpilogue };
