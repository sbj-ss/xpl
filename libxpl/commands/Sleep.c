#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcommand.h>
#include <libxpl/xplmessages.h>
#include <stdio.h>

void xplCmdSleepEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdSleepParams
{
	int delay;
} xplCmdSleepParams, *xplCmdSleepParamsPtr;

static const xplCmdSleepParams params_stencil =
{
	.delay = 0
};

xplCommand xplSleepCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdSleepEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdSleepParams),
	.parameters = {
		{
			.name = BAD_CAST "delay",
			.type = XPL_CMD_PARAM_TYPE_INT,
			.required = true,
			.value_stencil = &params_stencil.delay
		}, {
			.name = NULL
		}
	}
};

void xplCmdSleepEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSleepParamsPtr params = (xplCmdSleepParamsPtr) commandInfo->params;

	xprSleep(params->delay);
	ASSIGN_RESULT(NULL, false, true);
}
