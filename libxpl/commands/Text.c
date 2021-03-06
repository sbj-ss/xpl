#include <libxpl/xplcore.h>
#include <libxpl/xpltree.h>

void xplCmdTextPrologue(xplCommandInfoPtr commandInfo);
void xplCmdTextEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);
void xplCmdTextRestoreState(xplCommandInfoPtr commandInfo);

typedef struct _xplCmdTextParams
{
	bool repeat;
} xplCmdTextParams, *xplCmdTextParamsPtr;

static const xplCmdTextParams params_stencil =
{
	.repeat = false
};

xplCommand xplTextCommand = {
	.prologue = xplCmdTextPrologue,
	.epilogue = xplCmdTextEpilogue,
	.restore_state = xplCmdTextRestoreState,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdTextParams),
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

void xplCmdTextPrologue(xplCommandInfoPtr commandInfo)
{
	commandInfo->document->indent_spin++;
}

void xplCmdTextEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdTextParamsPtr params = (xplCmdTextParamsPtr) commandInfo->params;

	ASSIGN_RESULT(xplDetachChildren(commandInfo->element), params->repeat, true);
}

void xplCmdTextRestoreState(xplCommandInfoPtr commandInfo)
{
	commandInfo->document->indent_spin--;
}
