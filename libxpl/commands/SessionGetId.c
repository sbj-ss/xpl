#include <libxpl/xplcore.h>

void xplCmdSessionGetIdEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdSessionGetIdParams
{
	bool local;
} xplCmdSessionGetIdParams, *xplCmdSessionGetIdParamsPtr;

static const xplCmdSessionGetIdParams params_stencil =
{
	.local = false
};

xplCommand xplSessionGetIdCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdSessionGetIdEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdSessionGetIdParams),
	.parameters = {
		{
			.name = BAD_CAST "local",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.local
		}, {
			.name = NULL
		}
	}
};

void xplCmdSessionGetIdEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSessionGetIdParamsPtr params = (xplCmdSessionGetIdParamsPtr) commandInfo->params;
	xmlNodePtr ret = NULL;
	xmlChar *id;

	if ((id = xplDocSessionGetId(commandInfo->document, params->local)))
	{
		ret = xmlNewDocText(commandInfo->document->document, NULL);
		ret->content = id;
	}
	ASSIGN_RESULT(ret, false, true);
}
