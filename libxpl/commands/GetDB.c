#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsession.h>
#include <libxpl/xpltree.h>

void xplCmdGetDBEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdGetDBParams
{
	xmlChar *name;
	xplQName response_tag_name;
	bool show_tags;
	bool repeat;
} xplCmdGetDBParams, *xplCmdGetDBParamsPtr;

static const xplCmdGetDBParams params_stencil =
{
	.name = NULL,
	.response_tag_name = { NULL, BAD_CAST "Database" },
	.show_tags = false,
	.repeat = true
};

xplCommand xplGetDBCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdGetDBEpilogue,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdGetDBParams),
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.name
		}, {
			.name = BAD_CAST "responsetagname",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.value_stencil = &params_stencil.response_tag_name
		}, {
			.name = BAD_CAST "showtags",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.show_tags
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};

void xplCmdGetDBEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdGetDBParamsPtr cmd_params = (xplCmdGetDBParamsPtr) commandInfo->params;
	xplDBListPtr db_list;
	xmlNodePtr ret;

	if (!xplSessionGetSaMode(commandInfo->document->session))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), true, true);
		return;
	}

	if (cmd_params->name)
	{
		db_list = xplLocateDBList(cmd_params->name);
		if (!db_list)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid database name \"%s\"", cmd_params->name), true, true);
			return;
		}
		ret = xmlNewDocText(commandInfo->element->doc, db_list->conn_string);
		cmd_params->repeat = false;
	} else
		ret = xplDatabasesToNodeList(commandInfo->element, cmd_params->response_tag_name, cmd_params->show_tags);
	ASSIGN_RESULT(ret, cmd_params->repeat, true);
}
