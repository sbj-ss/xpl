#include <libxpl/xplcore.h>
#include "commands/WaitForThreads.h"

void xplCmdWaitForThreadsEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (commandInfo->document->threads)
		xplWaitForChildThreads(commandInfo->document);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplWaitForThreadsCommand = { NULL, xplCmdWaitForThreadsEpilogue };
