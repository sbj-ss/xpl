#include <libxpl/xplcommand.h>
#include "commands/ModuleLoaded.h"

typedef struct _xplCmdModuleLoadedParams
{
	xmlChar *name;
	xplQName tagname;
	bool show_tags;
	bool repeat;
} xplCmdModuleLoadedParams, *xplCmdModuleLoadedParamsPtr;

static const xplCmdModuleLoadedParams params_stencil =
{
	.name = NULL,
	.tagname = { NULL, BAD_CAST "module" },
	.show_tags = false,
	.repeat = true
};

xplCommand xplModuleLoadedCommand =
{
	.prologue = xplCmdModuleLoadedPrologue,
	.epilogue = xplCmdModuleLoadedEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdModuleLoadedParams),
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

void xplCmdModuleLoadedPrologue(xplCommandInfoPtr commandInfo)
{
	UNUSED_PARAM(commandInfo);
}

static const xplQName empty_qname = { NULL, NULL };

void xplCmdModuleLoadedEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdModuleLoadedParamsPtr params = (xplCmdModuleLoadedParamsPtr) commandInfo->params;
	xmlNodePtr ret;

	if (params->name)
	{
		ret = xmlNewDocText(commandInfo->element->doc, xplIsModuleLoaded(params->name)? BAD_CAST "true": BAD_CAST "false");
		ASSIGN_RESULT(ret, false, true);
	} else {
		ret = xplLoadedModulesToNodeList(params->show_tags? empty_qname: params->tagname, commandInfo->element);
		ASSIGN_RESULT(ret, params->repeat, true);
	}
}
