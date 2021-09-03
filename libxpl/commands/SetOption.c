#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>

void xplCmdSetOptionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdSetOptionParams
{
	xmlChar *name;
	bool to_default;
} xplCmdSetOptionParams, *xplCmdSetOptionParamsPtr;

static const xplCmdSetOptionParams params_stencil =
{
	.name = NULL,
	.to_default = false
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
			.name = BAD_CAST "todefault",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.to_default
		}, {
			.name = NULL
		}
	}
};

void xplCmdSetOptionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSetOptionParamsPtr params = (xplCmdSetOptionParamsPtr) commandInfo->params;

	if (!xplDocSessionGetSaMode(commandInfo->document))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "access denied"), true, true);
		return;
	}

	xplLockThreads(true);
	switch (xplSetOptionValue(params->name, commandInfo->content, params->to_default))
	{
		case XPL_SET_OPTION_OK:
			ASSIGN_RESULT(NULL, false, true);
			break;
		case XPL_SET_OPTION_UNKNOWN_OPTION:
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "unknown option \"%s\"", params->name), true, true);
			break;
		case XPL_SET_OPTION_INVALID_VALUE:
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "invalid option value \"%s\"", commandInfo->content), true, true);
			break;
		case XPL_SET_OPTION_INTERNAL_ERROR:
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "internal error, please contact the developer"), true, true);
			break;
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "internal error: unknown xplSetOptionValue() return code"), true, true);
	}
	xplLockThreads(false);
}
