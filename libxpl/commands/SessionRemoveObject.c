#include <libxpl/xplcore.h>

void xplCmdSessionRemoveObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdSessionRemoveObjectParams
{
	xmlChar *name;
	bool local;
} xplCmdSessionRemoveObjectParams, *xplCmdSessionRemoveObjectParamsPtr;

static const xplCmdSessionRemoveObjectParams params_stencil =
{
	.name = NULL,
	.local = false
};

xplCommand xplSessionRemoveObjectCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdSessionRemoveObjectEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdSessionRemoveObjectParams),
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_NCNAME,
			.required = true,
			.value_stencil = &params_stencil.name
		}, {
			.name = BAD_CAST "local",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.local
		}, {
			.name = NULL
		}
	}
};

void xplCmdSessionRemoveObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSessionRemoveObjectParamsPtr params = (xplCmdSessionRemoveObjectParamsPtr) commandInfo->params;

	xplDocSessionRemoveObject(commandInfo->document, params->local, params->name);
	ASSIGN_RESULT(NULL, false, true);
}
