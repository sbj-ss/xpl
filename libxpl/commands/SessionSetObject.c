#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>

void xplCmdSessionSetObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdSessionSetObjectParams
{
	xmlChar *name;
	bool local;
} xplCmdSessionSetObjectParams, *xplCmdSessionSetObjectParamsPtr;

static const xplCmdSessionSetObjectParams params_stencil =
{
	.name = NULL,
	.local = false
};

xplCommand xplSessionSetObjectCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdSessionSetObjectEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdSessionSetObjectParams),
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

void xplCmdSessionSetObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSessionSetObjectParamsPtr params = (xplCmdSessionSetObjectParamsPtr) commandInfo->params;

	if (!*params->name)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "name attribute is empty"), true, true);
		return;
	}
	xplDocSessionSetObject(commandInfo->document, params->local, commandInfo->element, params->name);
	ASSIGN_RESULT(NULL, false, true);
}
