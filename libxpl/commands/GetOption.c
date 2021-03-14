#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdGetOptionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdGetOptionParams
{
	xmlChar *name;
	xplQName tag_name;
	bool show_tags;
	bool show_passwords;
	bool repeat;
} xplCmdGetOptionParams, *xplCmdGetOptionParamsPtr;

static const xplCmdGetOptionParams params_stencil =
{
	.name = NULL,
	.tag_name = { NULL, NULL },
	.show_tags = false,
	.show_passwords = false,
	.repeat = true
};

xplCommand xplGetOptionCommand =
{
	.prologue = NULL,
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
			.name = BAD_CAST "tagname",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.value_stencil = &params_stencil.tag_name
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

static xplQName default_tag_name = { NULL, BAD_CAST "option" };
static xplQName empty_qname = { NULL, NULL };

void xplCmdGetOptionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdGetOptionParamsPtr params = (xplCmdGetOptionParamsPtr) commandInfo->params;
	xplQName tag_name;
	xmlNodePtr ret;
	bool found;
	xmlChar *value;

	if (params->show_passwords && !xplDocSessionGetSaMode(commandInfo->document))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "access denied"), true, true);
		return;
	}

	if (params->name)
	{
		value = xplGetOptionValue(params->name, params->show_passwords, &found);
		if (!found)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "option '%s' not found", params->name), true, true);
			return;
		}
		ret = xmlNewDocText(commandInfo->element->doc, NULL);
		ret->content = value;
		params->repeat = false;
	} else {
		tag_name = params->show_tags? empty_qname: params->tag_name.ncname? params->tag_name: default_tag_name;
		ret = xplOptionsToList(commandInfo->element, tag_name, params->show_passwords);
	}
	ASSIGN_RESULT(ret, params->repeat, true);
}
