#include <libxpl/xplcore.h>
#include <libxpl/xpltree.h>

void xplCmdStackLocalizePrologue(xplCommandInfoPtr commandInfo);
void xplCmdStackLocalizeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);
void xplCmdStackLocalizeRestoreState(xplCommandInfoPtr commandInfo);

typedef struct _xplCmdStackLocalizeParams
{
	bool repeat;
} xplCmdStackLocalizeParams, *xplCmdStackLocalizeParamsPtr;

static const xplCmdStackLocalizeParams params_stencil =
{
	.repeat = false
};

xplCommand xplStackLocalizeCommand = {
	.prologue = xplCmdStackLocalizePrologue,
	.epilogue = xplCmdStackLocalizeEpilogue,
	.restore_state = xplCmdStackLocalizeRestoreState,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdStackLocalizeParams),
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

void xplCmdStackLocalizePrologue(xplCommandInfoPtr commandInfo)
{
	commandInfo->prologue_state = commandInfo->document->stack;
	commandInfo->document->stack = NULL;
}

void xplCmdStackLocalizeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdStackLocalizeParamsPtr params = (xplCmdStackLocalizeParamsPtr) commandInfo->params;

	ASSIGN_RESULT(xplDetachChildren(commandInfo->element), params->repeat, true);
}

void xplCmdStackLocalizeRestoreState(xplCommandInfoPtr commandInfo)
{
	xmlNodePtr old_stack = (xmlNodePtr) commandInfo->prologue_state;

	xplClearDocStack(commandInfo->document);
	commandInfo->document->stack = old_stack;

}
