#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsession.h>
#include <libxpl/xpltree.h>

void xplCmdAddDBEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdAddDBParams
{
	xmlChar *name;
	bool check;
	bool modify_if_exists;
} xplCmdAddDBParams, *xplCmdAddDBParamsPtr;

static const xplCmdAddDBParams params_stencil =
{
	.name = NULL,
	.check = false,
	.modify_if_exists = false
};

xplCommand xplAddDBCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdAddDBEpilogue,
	.stencil_size = sizeof(xplCmdAddDBParams),
	.params_stencil = &params_stencil,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE | XPL_CMD_FLAG_REQUIRE_CONTENT,
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.name
		}, {
			.name = BAD_CAST "check",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.check
		}, {
			.name = BAD_CAST "modifyifexists",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.modify_if_exists
		}, {
			.name = NULL
		}
	}
};

void xplCmdAddDBEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdAddDBParamsPtr cmd_params = (xplCmdAddDBParamsPtr) commandInfo->params;
	xplDBConfigResult cfg_result;

	if (!xplSessionGetSaMode(commandInfo->document->session))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), true, true);
		return;
	}
	xplLockThreads(true);
	cfg_result = xplAddDB(cmd_params->name, commandInfo->content, cmd_params->check);
	if (cfg_result == XPL_DBCR_ALREADY_EXISTS && cmd_params->modify_if_exists)
		cfg_result = xplChangeDB(cmd_params->name, commandInfo->content, cmd_params->check);
	xplLockThreads(false);
	if (cfg_result == XPL_DBCR_OK)
		ASSIGN_RESULT(NULL, false, true);
	else
		ASSIGN_RESULT(xplCreateErrorNode(
			commandInfo->element, BAD_CAST "can't add database \"%s\": %s", cmd_params->name, xplDecodeDBConfigResult(cfg_result)
		), true, true);
}
