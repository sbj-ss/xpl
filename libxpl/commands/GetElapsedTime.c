#include <libxpl/xplcore.h>
#include <libxpl/abstraction/xpr.h>

void xplCmdGetElapsedTimeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdGetElapsedTimeParams
{
	bool restart_measurement;
} xplCmdGetElapsedTimeParams, *xplCmdGetElapsedTimeParamsPtr;

static const xplCmdGetElapsedTimeParams params_stencil =
{
	.restart_measurement = true
};

xplCommand xplGetElapsedTimeCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdGetElapsedTimeEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdGetElapsedTimeParams),
	.parameters = {
		{
			.name = BAD_CAST "restartmeasurement",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.restart_measurement
		}, {
			.name = NULL
		}
	}
};

void xplCmdGetElapsedTimeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdGetElapsedTimeParamsPtr params = (xplCmdGetElapsedTimeParamsPtr) commandInfo->params;
	xmlChar buf[32];
	xmlNodePtr ret;
	XPR_TIME now;
	long elapsed;

	xprGetTime(&now);
	elapsed = xprTimeDelta(&now, &commandInfo->document->profile_start_time);
	sprintf((char*) buf, "%ld", elapsed);
	ret = xmlNewDocText(commandInfo->document->document, buf);
	ASSIGN_RESULT(ret, false, true);
	if (params->restart_measurement)
		xprGetTime(&commandInfo->document->profile_start_time);
}
