#include <libxpl/xplcore.h>

void xplCmdWaitForThreadsEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplWaitForThreadsCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdWaitForThreadsEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdWaitForThreadsEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (commandInfo->document->threads)
		xplWaitForChildThreads(commandInfo->document);
	ASSIGN_RESULT(NULL, false, true);
}
