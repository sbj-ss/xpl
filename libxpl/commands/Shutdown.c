#include <stdlib.h>
#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>

void xplCmdShutdownEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdShutdownParams
{
	int code;
} xplCmdShutdownParams, *xplCmdShutdownParamsPtr;

static const xplCmdShutdownParams params_stencil =
{
	.code = EXIT_SUCCESS
};

xplCommand xplShutdownCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdShutdownEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.parameters = {
		{
			.name = BAD_CAST "code",
			.type = XPL_CMD_PARAM_TYPE_INT,
			.value_stencil = &params_stencil.code
		}, {
			.name = NULL
		}
	}
};

void xplCmdShutdownEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdShutdownParamsPtr params = (xplCmdShutdownParamsPtr) commandInfo->params;

	if (xplDocSessionGetSaMode(commandInfo->document))
		xprShutdownApp(params->code);
	else
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), true, true);
}
