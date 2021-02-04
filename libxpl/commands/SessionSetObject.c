#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsession.h>
#include <libxpl/xplstring.h>

void xplCmdSessionSetObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdSessionSetObjectParams
{
	xmlChar *name;
	bool thread_local;
} xplCmdSessionSetObjectParams, *xplCmdSessionSetObjectParamsPtr;

static const xplCmdSessionSetObjectParams params_stencil =
{
	.name = NULL,
	.thread_local = false
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
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.required = true,
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

void xplCmdSessionSetObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSessionSetObjectParamsPtr params = (xplCmdSessionSetObjectParamsPtr) commandInfo->params;

	if (!*params->name)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "name attribute is empty"), true, true);
		return;
	}
	if (!commandInfo->document->session)
		commandInfo->document->session = xplSessionCreateWithAutoId();
	if (params->thread_local)
		params->name = xstrAppendThreadIdToString(params->name, xprGetCurrentThreadId());
	xplSessionSetObject(commandInfo->document->session, commandInfo->element, params->name);
	ASSIGN_RESULT(NULL, false, true);
}
