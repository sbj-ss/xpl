#include "commands/StackPush.h"
#include "Core.h"
#include "Utils.h"

void xplCmdStackPushPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdStackPushEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplPushToDocStack(commandInfo->document, commandInfo->element);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplStackPushCommand = { xplCmdStackPushPrologue, xplCmdStackPushEpilogue };
