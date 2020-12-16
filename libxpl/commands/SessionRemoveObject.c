#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsession.h>
#include <libxpl/xplstring.h>

void xplCmdSessionRemoveObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdSessionRemoveObjectParams
{
	xmlChar *name;
	bool thread_local;
} xplCmdSessionRemoveObjectParams, *xplCmdSessionRemoveObjectParamsPtr;

static const xplCmdSessionRemoveObjectParams params_stencil =
{
	.name = NULL,
	.thread_local = false
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

void xplCmdSessionRemoveObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSessionRemoveObjectParamsPtr params = (xplCmdSessionRemoveObjectParamsPtr) commandInfo->params;

	if (params->thread_local)
		params->name = xstrAppendThreadIdToString(params->name, xprGetCurrentThreadId());
	xplSessionRemoveObject(commandInfo->document->main->session, params->name);
	ASSIGN_RESULT(NULL, false, true);
}
