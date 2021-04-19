#include <libxpl/xplcore.h>
#include <libxpl/xpldb.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdGetDBEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdGetDBParams
{
	xmlChar *name;
	xplQName tag_name;
	bool show_tags;
	bool repeat;
} xplCmdGetDBParams, *xplCmdGetDBParamsPtr;

static const xplCmdGetDBParams params_stencil =
{
	.name = NULL,
	.tag_name = { NULL, NULL },
	.show_tags = false,
	.repeat = true
};

static xmlChar* tagname_aliases[] = { BAD_CAST "responsetagname", NULL };

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
			.type = XPL_CMD_PARAM_TYPE_NCNAME,
			.value_stencil = &params_stencil.name
		}, {
			.name = BAD_CAST "tagname",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.aliases = tagname_aliases,
			.value_stencil = &params_stencil.tag_name
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

static const xplQName empty_qname = { NULL, NULL };
static const xplQName default_qname = { NULL, BAD_CAST "database" };

void xplCmdGetDBEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdGetDBParamsPtr params = (xplCmdGetDBParamsPtr) commandInfo->params;
	xplDBListPtr db_list;
	xmlNodePtr ret;
	xplQName tagname;

	if (params->tag_name.ncname && params->show_tags)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "showtags and tagname can't be used simultaneously"), true, true);
		return;
	}
	if (!xplDocSessionGetSaMode(commandInfo->document))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "access denied"), true, true);
		return;
	}

	if (params->name)
	{
		db_list = xplLocateDBList(params->name);
		if (!db_list)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "invalid database name \"%s\"", params->name), true, true);
			return;
		}
		ret = xmlNewDocText(commandInfo->element->doc, db_list->conn_string);
		params->repeat = false;
	} else {
		tagname = params->show_tags? empty_qname: params->tag_name.ncname? params->tag_name: default_qname;
		ret = xplDatabasesToNodeList(commandInfo->element, tagname);
	}
	ASSIGN_RESULT(ret, params->repeat, true);
}
