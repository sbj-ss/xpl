#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>

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
	if (!commandInfo->document->thread_handles)
	{
		if (cfgWarnOnNoAwaitableThreads)
			xplDisplayWarning(commandInfo->element, BAD_CAST "no awaitable threads");
	} else
		xplWaitForChildThreads(commandInfo->document);
	ASSIGN_RESULT(NULL, false, true);
}
