#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsession.h>
#include <libxpl/xpltree.h>
#include "commands/ChangeDB.h"

typedef struct _xplCmdChangeDBParams
{
	xmlChar *name;
	bool check;
	bool add_if_not_exists;
} xplCmdChangeDBParams, *xplCmdChangeDBParamsPtr;

static const xplCmdChangeDBParams params_stencil =
{
	.name = NULL,
	.check = false,
	.add_if_not_exists = false
};

xplCommand xplChangeDBCommand =
{
	.prologue = xplCmdChangeDBPrologue,
	.epilogue = xplCmdChangeDBEpilogue,
	.stencil_size = sizeof(xplCmdChangeDBParams),
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
			.name = BAD_CAST "addifnotexists",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.add_if_not_exists
		}, {
			.name = NULL
		}
	}
};

void xplCmdChangeDBPrologue(xplCommandInfoPtr commandInfo)
{

}

void xplCmdChangeDBEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdChangeDBParamsPtr cmd_params = (xplCmdChangeDBParamsPtr) commandInfo->params;
	xplDBConfigResult cfg_result;

	if (!xplSessionGetSaMode(commandInfo->document->session))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), true, true);
		return;
	}
	xplLockThreads(true);
	cfg_result = xplChangeDB(cmd_params->name, commandInfo->content, cmd_params->check);
	if (cfg_result == XPL_DBCR_NOT_FOUND && cmd_params->add_if_not_exists)
		cfg_result = xplAddDB(cmd_params->name, commandInfo->content, cmd_params->check);
	xplLockThreads(false);
	if (cfg_result == XPL_DBCR_OK)
		ASSIGN_RESULT(NULL, false, true);
	else
		ASSIGN_RESULT(xplCreateErrorNode(
			commandInfo->element, BAD_CAST "can't modify database \"%s\": %s", cmd_params->name, xplDecodeDBConfigResult(cfg_result)
		), true, true);
}
