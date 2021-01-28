#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsession.h>

void xplCmdSetSaModeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdSetSaModeParams
{
	bool enable;
	xmlChar *password;
} xplCmdSetSaModeParams, *xplCmdSetSaModeParamsPtr;

static const xplCmdSetSaModeParams params_stencil =
{
	.enable = true,
	.password = NULL
};

xplCommand xplSetSaModeCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdSetSaModeEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdSetSaModeParams),
	.parameters = {
		{
			.name = BAD_CAST "enable",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.enable
		}, {
			.name = BAD_CAST "password",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.password
		}, {
			.name = NULL
		}
	}
};

void xplCmdSetSaModeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSetSaModeParamsPtr params = (xplCmdSetSaModeParamsPtr) commandInfo->params;

	if (!commandInfo->document->session)
		commandInfo->document->session = xplSessionCreateWithAutoId();

	if (params->enable && !params->password)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "password is required to enable the SA mode"), true, true);
		return;
	}

	if (xplSessionSetSaMode(commandInfo->document->session, params->enable, params->password))
		ASSIGN_RESULT(NULL, false, true);
	else
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "the password is incorrect"), true, true);
}
