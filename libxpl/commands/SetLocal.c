#include <libxpl/xplcore.h>
#include <libxpl/xplparams.h>
#include <libxpl/xpltree.h>

void xplCmdSetLocalPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSetLocalEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);
void xplCmdSetLocalRestoreState(xplCommandInfoPtr commandInfo);

typedef struct _xplCmdSetLocalParams
{
	bool repeat;
} xplCmdSetLocalParams, *xplCmdSetLocalParamsPtr;

static const xplCmdSetLocalParams params_stencil =
{
	.repeat = false
};

xplCommand xplSetLocalCommand = {
	.prologue = xplCmdSetLocalPrologue,
	.epilogue = xplCmdSetLocalEpilogue,
	.restore_state = xplCmdSetLocalRestoreState,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdSetLocalParams),
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

void xplCmdSetLocalPrologue(xplCommandInfoPtr commandInfo)
{
	xplParamsPtr old_params = commandInfo->document->params;
	xplParamsPtr tmp_params = NULL;

	if (old_params)
	{
		tmp_params = xplParamsCopy(old_params);
		commandInfo->document->params = tmp_params;
	}
	commandInfo->prologue_state = old_params;
}

void xplCmdSetLocalEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSetLocalParamsPtr params = (xplCmdSetLocalParamsPtr) commandInfo->params;

	ASSIGN_RESULT(xplDetachChildren(commandInfo->element), params->repeat, true);
}

void xplCmdSetLocalRestoreState(xplCommandInfoPtr commandInfo)
{
	xplParamsPtr old_params, tmp_params;
	
	tmp_params = commandInfo->document->params;
	if (tmp_params)
		xplParamsFree(tmp_params);
	old_params = (xplParamsPtr) commandInfo->prologue_state;
	commandInfo->document->params = old_params;
}
