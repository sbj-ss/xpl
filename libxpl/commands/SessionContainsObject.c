#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplsession.h>
#include <libxpl/xplstring.h>

void xplCmdSessionContainsObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdSessionContainsObjectParams
{
	xmlChar *name;
	bool thread_local;
} xplCmdSessionContainsObjectParams, *xplCmdSessionContainsObjectParamsPtr;

static const xplCmdSessionContainsObjectParams params_stencil =
{
	.name = NULL,
	.thread_local = false
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
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.name
		}, {
			.name = BAD_CAST "threadlocal",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.thread_local
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

	ret_value = (commandInfo->document->session && xplSessionIsValid(commandInfo->document->session));
	if (params->name)
	{
		if (params->thread_local)
			params->name = xstrAppendThreadIdToString(params->name, xprGetCurrentThreadId());
		ret_value = ret_value && xplSessionGetObject(commandInfo->document->session, params->name);
	}
	ret = xmlNewDocText(commandInfo->document->document, ret_value? BAD_CAST "true": BAD_CAST "false");
	ASSIGN_RESULT(ret, false, true);	
}
