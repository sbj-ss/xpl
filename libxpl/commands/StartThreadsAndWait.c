#include <libxpl/xplcore.h>

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

	if (commandInfo->document->has_suspended_threads)
	{
		xplStartDelayedThreads(commandInfo->document);
		xplWaitForChildThreads(commandInfo->document);
	}
	ASSIGN_RESULT(xplDetachChildren(commandInfo->element), params->repeat, true);
}
