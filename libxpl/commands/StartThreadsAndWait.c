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
	bool launch_ok = true;

	if (!commandInfo->document->suspended_thread_docs)
	{
		if (cfgWarnOnNoAwaitableThreads)
			xplDisplayWarning(commandInfo->element, "no suspended threads to start");
	} else {
		launch_ok = xplStartDelayedThreads(commandInfo->document);
		xplWaitForChildThreads(commandInfo->document);
	}
	if (launch_ok)
		ASSIGN_RESULT(xplDetachChildren(commandInfo->element), params->repeat, true);
	else
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "launching some threads failed"), true, true);

}
