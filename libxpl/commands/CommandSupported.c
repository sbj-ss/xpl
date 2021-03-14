#include <libxpl/xplcommand.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdCommandSupportedEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdCommandSupportedParams
{
	xmlChar *name;
	xplQName tagname;
	bool repeat;
	bool show_tags;
} xplCmdCommandSupportedParams, *xplCmdCommandSupportedParamsPtr;

static const xplCmdCommandSupportedParams params_stencil =
{
	.name = NULL,
	.tagname = { NULL, NULL },
	.repeat = true,
	.show_tags = false
};

xplCommand xplCommandSupportedCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdCommandSupportedEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdCommandSupportedParams),
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.name
		}, {
			.name = BAD_CAST "tagname",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.value_stencil = &params_stencil.tagname
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = BAD_CAST "showtags",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.show_tags
		}, {
			.name = NULL
		}
	}
};

static const xplQName empty_qname = { NULL, NULL };
static const xplQName default_qname = { NULL, BAD_CAST "command" };

void xplCmdCommandSupportedEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdCommandSupportedParamsPtr params = (xplCmdCommandSupportedParamsPtr) commandInfo->params;
	xmlChar *value;
	xplQName tagname;
	xmlNodePtr ret;

	if (!params->name) /* all commands */
	{
		if (params->show_tags && params->tagname.ncname)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "tagname and showtags can't be used simultaneously"), true, true);
			return;
		}
		tagname = params->show_tags? empty_qname: params->tagname.ncname? params->tagname: default_qname;
		ASSIGN_RESULT(xplSupportedCommandsToList(commandInfo->element, tagname), params->repeat, true);
	} else { /* specified command */
		if (params->tagname.ncname || params->show_tags)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "name can't be used together with tagname or showtags"), true, true);
			return;
		}
		value = BAD_CAST (xplCommandSupported(params->name)? "true": "false");
		ret = xmlNewDocText(commandInfo->element->doc, value);
		ASSIGN_RESULT(ret, false, true);
	}
}
