#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplsession.h>
#include <libxpl/xpltree.h>
#include "commands/GetOption.h"

typedef struct _xplCmdGetOptionParams
{
	xmlChar *name;
	xplQName response_tag_name;
	bool show_tags;
	bool show_passwords;
	bool repeat;
} xplCmdGetOptionParams, *xplCmdGetOptionParamsPtr;

static const xplCmdGetOptionParams params_stencil =
{
	.name = NULL,
	.response_tag_name = { NULL, BAD_CAST "option" },
	.show_tags = false,
	.show_passwords = false,
	.repeat = true
};

xplCommand xplGetOptionCommand =
{
	.prologue = xplCmdGetOptionPrologue,
	.epilogue = xplCmdGetOptionEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdGetOptionParams),
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
			.name = BAD_CAST "showpasswords",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.show_passwords
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};

void xplCmdGetOptionPrologue(xplCommandInfoPtr commandInfo)
{
	UNUSED_PARAM(commandInfo);
}

void xplCmdGetOptionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdGetOptionParamsPtr params = (xplCmdGetOptionParamsPtr) commandInfo->params;
	xmlNodePtr ret;

	if (params->show_passwords && !xplSessionGetSaMode(commandInfo->document->session))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), true, true);
		return;
	}

	if (params->name)
	{
		ret = xmlNewDocText(commandInfo->element->doc, NULL);
		ret->content = xplGetOptionValue(params->name, params->show_passwords);
		params->repeat = false;
	} else {
		ret = xplOptionsToList(commandInfo->element, params->response_tag_name, params->show_tags, params->show_passwords);
		xplDownshiftNodeListNsDef(ret, commandInfo->element->nsDef);
	}
	ASSIGN_RESULT(ret, params->repeat, true);
}
