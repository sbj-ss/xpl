#include <libxpl/xpltree.h>
#include "commands/CommandSupported.h"

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
	.tagname = { NULL, BAD_CAST "command" },
	.repeat = true,
	.show_tags = false
};

xplCommand xplCommandSupportedCommand =
{
	.prologue = xplCmdCommandSupportedPrologue,
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

void xplCmdCommandSupportedPrologue(xplCommandInfoPtr commandInfo)
{
	UNUSED_PARAM(commandInfo);
}

static const xplQName empty_qname = { NULL, NULL };

void xplCmdCommandSupportedEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdCommandSupportedParamsPtr params = (xplCmdCommandSupportedParamsPtr) commandInfo->params;
	xmlChar *value;
	xmlNodePtr ret;

	if (!params->name) /* all commands */
	{
		ASSIGN_RESULT(xplSupportedCommandsToList(commandInfo->element, params->show_tags? empty_qname: params->tagname), params->repeat, true);
	} else { /* specified command */
		value = BAD_CAST (xplCommandSupported(params->name)? "true": "false");
		ret = xmlNewDocText(commandInfo->element->doc, value);
		ASSIGN_RESULT(ret, false, true);
	}
}
