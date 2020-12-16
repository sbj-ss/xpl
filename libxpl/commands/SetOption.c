#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplsession.h>

void xplCmdSetOptionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdSetOptionParams
{
	xmlChar *name;
} xplCmdSetOptionParams, *xplCmdSetOptionParamsPtr;

static const xplCmdSetOptionParams params_stencil =
{
	.name = NULL,
};

xplCommand xplSetOptionCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdSetOptionEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdSetOptionParams),
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.required = true,
			.value_stencil = &params_stencil.name
		}, {
			.name = NULL
		}
	}
};

void xplCmdSetOptionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSetOptionParamsPtr params = (xplCmdSetOptionParamsPtr) commandInfo->params;
	bool by_default = false;

	if (!xplSessionGetSaMode(commandInfo->document->session))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), true, true);
		return;
	}
	if (!commandInfo->content)
		by_default = true;

	xplLockThreads(true);
	switch (xplSetOptionValue(params->name, commandInfo->content, by_default))
	{
		case XPL_SET_OPTION_OK:
			ASSIGN_RESULT(NULL, false, true);
			break;
		case XPL_SET_OPTION_UNKNOWN_OPTION:
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "unknown option \"%s\"", params->name), true, true);
			break;
		case XPL_SET_OPTION_INVALID_VALUE:
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid option value \"%s\"", commandInfo->content), true, true);
			break;
		case XPL_SET_OPTION_INTERNAL_ERROR:
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "internal error, please contact the developer"), true, true);
			break;
	}
	xplLockThreads(false);
}
