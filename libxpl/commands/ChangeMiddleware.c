#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplmiddleware.h>
#include <libxpl/xpltree.h>

void xplCmdChangeMiddlewareEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdChangeMiddlewareParams
{
	xmlChar *regex;
	xmlChar *file;
	bool add_if_not_exists;
} xplCmdChangeMiddlewareParams, *xplCmdChangeMiddlewareParamsPtr;

static const xplCmdChangeMiddlewareParams params_stencil =
{
	.regex = NULL,
	.file = NULL,
	.add_if_not_exists = false
};

xplCommand xplChangeMiddlewareCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdChangeMiddlewareEpilogue,
	.stencil_size = sizeof(xplCmdChangeMiddlewareParams),
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
			.name = BAD_CAST "addifnotexists",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.add_if_not_exists
		}, {
			.name = NULL
		}
	}
};

void xplCmdChangeMiddlewareEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdChangeMiddlewareParamsPtr params = (xplCmdChangeMiddlewareParamsPtr) commandInfo->params;
	xplMWResult cfg_result;

	if (!xplDocSessionGetSaMode(commandInfo->document))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "access denied"), true, true);
		return;
	}
	cfg_result = xplMWChangeEntry(params->regex, params->file, params->add_if_not_exists);
	if (cfg_result == XPL_MW_OK)
		ASSIGN_RESULT(NULL, false, true);
	else
		ASSIGN_RESULT(xplCreateErrorNode(
			commandInfo->element, "can't change middleware \"%s\": %s", params->regex, xplMWResultToString(cfg_result)
		), true, true);
}
