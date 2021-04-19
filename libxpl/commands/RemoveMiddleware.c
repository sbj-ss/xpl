#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplmiddleware.h>
#include <libxpl/xpltree.h>

void xplCmdRemoveMiddlewareEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdRemoveMiddlewareParams
{
	xmlChar *regex;
	bool ignore_if_not_exists;
} xplCmdRemoveMiddlewareParams, *xplCmdRemoveMiddlewareParamsPtr;

static const xplCmdRemoveMiddlewareParams params_stencil =
{
	.regex = NULL,
	.ignore_if_not_exists = false
};

xplCommand xplRemoveMiddlewareCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdRemoveMiddlewareEpilogue,
	.stencil_size = sizeof(xplCmdRemoveMiddlewareParams),
	.params_stencil = &params_stencil,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.parameters = {
		{
			.name = BAD_CAST "regex",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.required = true,
			.value_stencil = &params_stencil.regex
		}, {
			.name = BAD_CAST "ignoreifnotexists",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.ignore_if_not_exists
		}, {
			.name = NULL
		}
	}
};

void xplCmdRemoveMiddlewareEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdRemoveMiddlewareParamsPtr params = (xplCmdRemoveMiddlewareParamsPtr) commandInfo->params;
	xplMWResult cfg_result;

	if (!xplDocSessionGetSaMode(commandInfo->document))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "access denied"), true, true);
		return;
	}
	cfg_result = xplMWRemoveEntry(params->regex, params->ignore_if_not_exists);
	if (cfg_result == XPL_MW_OK)
		ASSIGN_RESULT(NULL, false, true);
	else
		ASSIGN_RESULT(xplCreateErrorNode(
			commandInfo->element, "can't remove middleware \"%s\": %s", params->regex, xplMWResultToString(cfg_result)
		), true, true);
}
