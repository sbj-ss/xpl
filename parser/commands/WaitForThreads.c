#include <libxpl/xplcore.h>
#include "commands/WaitForThreads.h"

void xplCmdWaitForThreadsPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdWaitForThreadsEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (commandInfo->document->threads)
		xplWaitForChildThreads(commandInfo->document);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplWaitForThreadsCommand = { xplCmdWaitForThreadsPrologue, xplCmdWaitForThreadsEpilogue };
