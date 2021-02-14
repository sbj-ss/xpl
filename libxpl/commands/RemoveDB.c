#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>

void xplCmdRemoveDBEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdRemoveDBParams
{
	xmlChar *name;
	bool ignore_if_not_exists;
} xplCmdRemoveDBParams, *xplCmdRemoveDBParamsPtr;

static const xplCmdRemoveDBParams params_stencil =
{
	.name = NULL,
	.ignore_if_not_exists = false
};

xplCommand xplRemoveDBCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdRemoveDBEpilogue,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdRemoveDBParams),
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_NCNAME,
			.required = true,
			.value_stencil = &params_stencil.name
		}, {
			.name = BAD_CAST "ignoreifnotexists",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.ignore_if_not_exists
		}, {
			.name = NULL
		}
	}
};

void xplCmdRemoveDBEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdRemoveDBParamsPtr params = (xplCmdRemoveDBParamsPtr) commandInfo->params;
	xplDBConfigResult cfg_result;

	if (!xplSessionGetSaMode(commandInfo->document->session))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), true, true);
		return;
	}
	xplLockThreads(true);
	cfg_result = xplRemoveDB(params->name);
	if (cfg_result == XPL_DBCR_NOT_FOUND && params->ignore_if_not_exists)
		cfg_result = XPL_DBCR_OK;
	xplLockThreads(false);
	if (cfg_result == XPL_DBCR_OK)
		ASSIGN_RESULT(NULL, false, true);
	else
		ASSIGN_RESULT(xplCreateErrorNode(
			commandInfo->element, BAD_CAST "can't remove database \"%s\": %s", params->name, xplDecodeDBConfigResult(cfg_result)
		), true, true);
}
