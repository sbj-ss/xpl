#include <libxpl/xplcore.h>
#include "commands/StackPush.h"

void xplCmdStackPushEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplPushToDocStack(commandInfo->document, commandInfo->element);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplStackPushCommand = { NULL, xplCmdStackPushEpilogue };
