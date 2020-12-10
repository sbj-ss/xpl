#include <libxpl/xplcore.h>

void xplCmdStartThreadsAndWaitEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

#define REPEAT_ATTR (BAD_CAST "repeat")
void xplCmdStartThreadsAndWaitEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	bool repeat;
	xmlNodePtr error;

	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, false)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	if (commandInfo->document->has_suspended_threads)
	{
		xplStartDelayedThreads(commandInfo->document);
		xplWaitForChildThreads(commandInfo->document);
	}
	ASSIGN_RESULT(xplDetachContent(commandInfo->element), repeat, true);
}

xplCommand xplStartThreadsAndWaitCommand = { NULL, xplCmdStartThreadsAndWaitEpilogue };
