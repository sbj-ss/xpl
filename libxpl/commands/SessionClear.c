#include <libxpl/xplcore.h>

void xplCmdSessionClearEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdSessionClearParams
{
	bool local;
} xplCmdSessionClearParams, *xplCmdSessionClearParamsPtr;

static const xplCmdSessionClearParams params_stencil =
{
	.local = false
};

xplCommand xplSessionClearCommand = {
	.prologue = NULL,
	.epilogue = xplCmdSessionClearEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdSessionClearParams),
	.parameters = {
		{
			.name = BAD_CAST "local",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.local
		}, {
			.name = NULL
		}
	}
};

void xplCmdSessionClearEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSessionClearParamsPtr params = (xplCmdSessionClearParamsPtr) commandInfo->params;
	xplDocSessionClear(commandInfo->document, params->local);
	ASSIGN_RESULT(NULL, false, true);
}
