#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcore.h>

void xplCmdSessionContainsObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdSessionContainsObjectParams
{
	xmlChar *name;
	bool local;
} xplCmdSessionContainsObjectParams, *xplCmdSessionContainsObjectParamsPtr;

static const xplCmdSessionContainsObjectParams params_stencil =
{
	.name = NULL,
	.local = false
};

xplCommand xplSessionContainsObjectCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdSessionContainsObjectEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdSessionContainsObjectParams),
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_NCNAME,
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

void xplCmdSessionContainsObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSessionContainsObjectParamsPtr params = (xplCmdSessionContainsObjectParamsPtr) commandInfo->params;
	bool ret_value;
	xmlNodePtr ret;

	ret_value = xplDocSessionExists(commandInfo->document, params->local);
	if (ret_value && params->name)
		ret_value = xplSessionGetObject(xplDocSessionGetOrCreate(commandInfo->document, params->local), params->name);
	ret = xmlNewDocText(commandInfo->document->document, ret_value? BAD_CAST "true": BAD_CAST "false");
	ASSIGN_RESULT(ret, false, true);	
}
