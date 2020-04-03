#include "commands/StartThreadsAndWait.h"
#include "Core.h"
#include "Utils.h"

void xplCmdStartThreadsAndWaitPrologue(xplCommandInfoPtr commandInfo)
{
}

#define REPEAT_ATTR (BAD_CAST "repeat")
void xplCmdStartThreadsAndWaitEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	BOOL repeat;
	xmlNodePtr error;

	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, FALSE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		return;
	}
	if (commandInfo->document->has_suspended_threads)
	{
		xplStartDelayedThreads(commandInfo->document);
		xplWaitForChildThreads(commandInfo->document);
	}
	ASSIGN_RESULT(detachContent(commandInfo->element), repeat, TRUE);
}

xplCommand xplStartThreadsAndWaitCommand = { xplCmdStartThreadsAndWaitPrologue, xplCmdStartThreadsAndWaitEpilogue };
