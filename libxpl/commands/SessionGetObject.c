#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsession.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>

void xplCmdSessionGetObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdSessionGetObjectParams
{
	xmlChar *name;
	xmlChar *select;
	bool repeat;
	bool local;
} xplCmdSessionGetObjectParams, *xplCmdSessionGetObjectParamsPtr;

static const xplCmdSessionGetObjectParams params_stencil =
{
	.name = NULL,
	.select = NULL,
	.repeat = true,
	.local = false
};

xplCommand xplSessionGetObjectCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdSessionGetObjectEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdSessionGetObjectParams),
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_NCNAME,
			.value_stencil = &params_stencil.name
		}, {
			.name = BAD_CAST "select",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.select
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = BAD_CAST "local",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.local
		}, {
			.name = NULL
		}
	}
};

void xplCmdSessionGetObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSessionGetObjectParamsPtr params = (xplCmdSessionGetObjectParamsPtr) commandInfo->params;
	xmlNodePtr ret;
	bool ok;

	if (params->name)
		ret = xplDocSessionGetObject(commandInfo->document, params->local, params->name, commandInfo->element, params->select, &ok);
	else
		ret = xplDocSessionGetAllObjects(commandInfo->document, params->local, commandInfo->element, params->select, &ok);
	ASSIGN_RESULT(ret, !ok || params->repeat, true);
}
