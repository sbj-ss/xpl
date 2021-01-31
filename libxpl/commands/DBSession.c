#include <libxpl/xplcommand.h>
#include <libxpl/xpltree.h>

void xplCmdDBSessionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdDBSessionParams
{
	bool repeat;
} xplCmdDBSessionParams, *xplCmdDBSessionParamsPtr;

static const xplCmdDBSessionParams params_stencil =
{
	.repeat = false
};

xplCommand xplDBSessionCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdDBSessionEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdDBSessionParams),
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

void xplCmdDBSessionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdDBSessionParamsPtr params = (xplCmdDBSessionParamsPtr) commandInfo->params;

	ASSIGN_RESULT(xplDetachChildren(commandInfo->element), params->repeat, true);
}
