#include <libxpl/xplcommand.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>

void xplCmdLoadModuleEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdLoadModuleParams
{
	xmlChar *name;
	bool fail_if_already_loaded;
} xplCmdLoadModuleParams, *xplCmdLoadModuleParamsPtr;

static const xplCmdLoadModuleParams params_stencil =
{
	.name = NULL,
	.fail_if_already_loaded = false
};

xplCommand xplLoadModuleCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdLoadModuleEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdLoadModuleParams),
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.required = true,
			.value_stencil = &params_stencil.name
		}, {
			.name = BAD_CAST "failifalreadyloaded",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.fail_if_already_loaded
		}, {
			.name = NULL
		}
	}
};

void xplCmdLoadModuleEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdLoadModuleParamsPtr params = (xplCmdLoadModuleParamsPtr) commandInfo->params;
	xmlChar *error;
	xmlNodePtr ret = NULL;
	xplModuleCmdResult code;

	xplLockThreads(true);
	code = xplLoadModule(params->name, &error);
	if ((code != XPL_MODULE_CMD_OK) || (code != XPL_MODULE_CMD_MODULE_ALREADY_LOADED))
	{
		ret = xplCreateErrorNode(commandInfo->element, BAD_CAST "cannot load module '%s': %s", params->name, error);
		if (error)
			XPL_FREE(error);
	} else if (code == XPL_MODULE_CMD_MODULE_ALREADY_LOADED && params->fail_if_already_loaded)
		ret = xplCreateErrorNode(commandInfo->element, BAD_CAST "module '%s' already loaded", params->name);
	xplLockThreads(false);
	ASSIGN_RESULT(ret, true, true);
}
