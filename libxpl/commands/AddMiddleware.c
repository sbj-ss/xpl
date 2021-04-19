#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplmiddleware.h>
#include <libxpl/xpltree.h>

void xplCmdAddMiddlewareEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdAddMiddlewareParams
{
	xmlChar *regex;
	xmlChar *file;
	bool modify_if_exists;
} xplCmdAddMiddlewareParams, *xplCmdAddMiddlewareParamsPtr;

static const xplCmdAddMiddlewareParams params_stencil =
{
	.regex = NULL,
	.file = NULL,
	.modify_if_exists = false
};

xplCommand xplAddMiddlewareCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdAddMiddlewareEpilogue,
	.stencil_size = sizeof(xplCmdAddMiddlewareParams),
	.params_stencil = &params_stencil,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.parameters = {
		{
			.name = BAD_CAST "regex",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.required = true,
			.value_stencil = &params_stencil.regex
		}, {
			.name = BAD_CAST "file",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.file
		}, {
			.name = BAD_CAST "modifyifexists",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.modify_if_exists
		}, {
			.name = NULL
		}
	}
};

void xplCmdAddMiddlewareEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdAddMiddlewareParamsPtr params = (xplCmdAddMiddlewareParamsPtr) commandInfo->params;
	xplMWResult cfg_result;

	if (!xplDocSessionGetSaMode(commandInfo->document))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "access denied"), true, true);
		return;
	}
	cfg_result = xplMWAddEntry(params->regex, params->file, params->modify_if_exists);
	if (cfg_result == XPL_MW_OK)
		ASSIGN_RESULT(NULL, false, true);
	else
		ASSIGN_RESULT(xplCreateErrorNode(
			commandInfo->element, "can't add middleware \"%s\": %s", params->regex, xplMWResultToString(cfg_result)
		), true, true);
}
