#include <libxpl/xplcommand.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdDebugPrintEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdDebugPrintParams
{
	xplMsgType severity;
} xplCmdDebugPrintParams, *xplCmdDebugPrintParamsPtr;

static const xplCmdDebugPrintParams params_stencil =
{
	.severity = XPL_MSG_DEBUG
};

static xmlChar* _getSeverity(xplCommandInfoPtr info, const xmlChar *raw_value, int *result)
{
	UNUSED_PARAM(info);
	if ((*result = xplMsgTypeFromString(raw_value, false)) == XPL_MSG_UNKNOWN)
		return xplFormatMessage(BAD_CAST "unknown severity value '%s'", raw_value);
	return NULL;
}

xplCommand xplDebugPrintCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdDebugPrintEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdDebugPrintParams),
	.parameters = {
		{
			.name = BAD_CAST "severity",
			.type = XPL_CMD_PARAM_TYPE_INT_CUSTOM_GETTER,
			.extra = {
				.int_getter = _getSeverity
			},
			.value_stencil = &params_stencil.severity
		}, {
			.name = NULL
		}
	}
};

void xplCmdDebugPrintEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdDebugPrintParamsPtr params = (xplCmdDebugPrintParamsPtr) commandInfo->params;

	if ((int) params->severity < cfgMinDebugPrintLevel) /* suppress unwanted noise */
	{
		ASSIGN_RESULT(NULL, false, true);
		return;
	}
	xplDisplayMessage(params->severity, BAD_CAST "%s", commandInfo->content? commandInfo->content: BAD_CAST "<no message provided>");
	ASSIGN_RESULT(NULL, true, true);
}
