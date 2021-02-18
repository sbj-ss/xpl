#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>

void xplCmdUnloadModuleEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdUnloadModuleParams
{
	xmlChar *name;
} xplCmdUnloadModuleParams, *xplCmdUnloadModuleParamsPtr;

static const xplCmdUnloadModuleParams params_stencil =
{
	.name = NULL
};

xplCommand xplUnloadModuleCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdUnloadModuleEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdUnloadModuleParams),
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

void xplCmdUnloadModuleEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdUnloadModuleParamsPtr params = (xplCmdUnloadModuleParamsPtr) commandInfo->params;

	xplLockThreads(true);
	xplUnloadModule(params->name); // ignore modules that haven't been loaded
	xplLockThreads(false);
	ASSIGN_RESULT(NULL, false, true);
}
