#include <libxpl/xplcore.h>

void xplCmdStackPushEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

void xplCmdStackPushEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplPushToDocStack(commandInfo->document, commandInfo->element);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplStackPushCommand = { NULL, xplCmdStackPushEpilogue };
