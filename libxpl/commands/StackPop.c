#include <libxpl/xplcore.h>

void xplCmdStackPopEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdStackPopParams
{
	bool repeat;
} xplCmdStackPopParams, *xplCmdStackPopParamsPtr;

static const xplCmdStackPopParams params_stencil =
{
	.repeat = true
};

xplCommand xplStackPopCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdStackPopEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdStackPopParams),
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

void xplCmdStackPopEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdStackPopParamsPtr params = (xplCmdStackPopParamsPtr) commandInfo->params;

	ASSIGN_RESULT(xplPopFromDocStack(commandInfo->document, commandInfo->element->parent), params->repeat, true);
}
