#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>

void xplCmdStartThreadsAndWaitEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdStartThreadsAndWaitParams
{
	bool repeat;
} xplCmdStartThreadsAndWaitParams, *xplCmdStartThreadsAndWaitParamsPtr;

static const xplCmdStartThreadsAndWaitParams params_stencil =
{
	.repeat = false
};

xplCommand xplStartThreadsAndWaitCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdStartThreadsAndWaitEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdStartThreadsAndWaitParams),
	.parameters = {
		{
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};

void xplCmdStartThreadsAndWaitEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdStartThreadsAndWaitParamsPtr params = (xplCmdStartThreadsAndWaitParamsPtr) commandInfo->params;

	if (!commandInfo->document->suspended_thread_docs)
	{
		if (cfgWarnOnNoAwaitableThreads)
			xplDisplayWarning(commandInfo->element, BAD_CAST "no suspended threads to start");
	} else {
		xplStartDelayedThreads(commandInfo->document);
		xplWaitForChildThreads(commandInfo->document);
	}
	ASSIGN_RESULT(xplDetachChildren(commandInfo->element), params->repeat, true);
}
