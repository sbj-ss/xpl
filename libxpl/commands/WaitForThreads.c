#include <libxpl/xplcore.h>

void xplCmdWaitForThreadsEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

void xplCmdWaitForThreadsEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (commandInfo->document->threads)
		xplWaitForChildThreads(commandInfo->document);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplWaitForThreadsCommand = { NULL, xplCmdWaitForThreadsEpilogue };
