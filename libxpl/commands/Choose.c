#include <libxpl/xplcommand.h>
#include <libxpl/xpltree.h>

void xplCmdChooseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdChooseParams
{
	bool repeat;
} xplCmdChooseParams, *xplCmdChooseParamsPtr;

static const xplCmdChooseParams params_stencil =
{
	.repeat = false
};

xplCommand xplChooseCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdChooseEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdChooseParams),
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

void xplCmdChooseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdChooseParamsPtr params = (xplCmdChooseParamsPtr) commandInfo->params;

	ASSIGN_RESULT(xplDetachChildren(commandInfo->element), params->repeat, true);
}
