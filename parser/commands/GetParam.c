#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplparams.h>
#include <libxpl/xpltree.h>
#include "commands/GetParam.h"

typedef struct _xplCmdGetParamParams
{
	xmlChar *default_value;
	xmlChar *delimiter;
	xplExpectType expect;
	xmlChar *name;
	bool repeat;
	xplQName response_tag_name;
	bool show_tags;
	int type;
	bool unique;
} xplCmdGetParamParams, *xplCmdGetParamParamsPtr;

static const xplCmdGetParamParams params_stencil =
{
	.default_value = NULL,
	.delimiter = BAD_CAST "",
	.expect = XPL_EXPECT_UNSPECIFIED,
	.name = NULL,
	.repeat = true,
	.response_tag_name = {},
	.show_tags = false,
	.type = XPL_PARAM_TYPE_USERDATA,
	.unique = false
};

xplCommand xplGetParamCommand =
{
	.prologue = xplCmdGetParamPrologue,
	.epilogue = xplCmdGetParamEpilogue,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdGetParamParams),
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.parameters = {
		{
			.name = BAD_CAST "default",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.default_value
		}, {
			.name = BAD_CAST "delimiter",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.delimiter,
		}, {
			.name = BAD_CAST "expect",
			.type = XPL_CMD_PARAM_TYPE_INT_CUSTOM_GETTER,
			.extra.int_getter = xplExpectTypeGetter,
			.value_stencil = &params_stencil.expect
		}, {
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.name
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = BAD_CAST "responsetagname",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.value_stencil = &params_stencil.response_tag_name
		}, {
			.name = BAD_CAST "showtags",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.show_tags
		}, {
			.name = BAD_CAST "type",
			.type = XPL_CMD_PARAM_TYPE_INT_CUSTOM_GETTER,
			.extra.int_getter = xplParamTypeMaskGetter,
			.value_stencil = &params_stencil.type
		}, {
			.name = BAD_CAST "unique",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.unique
		}, {
			.name = NULL
		}
	}
};

void xplCmdGetParamPrologue(xplCommandInfoPtr commandInfo)
{
}

static const xplQName default_qname = { NULL, BAD_CAST "param" };
static const xplQName empty_qname = { NULL, NULL };

void xplCmdGetParamEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdGetParamParamsPtr cmd_params = (xplCmdGetParamParamsPtr) commandInfo->params;
	xmlChar *txt;
	xplParamValuesPtr values;
	xmlNodePtr ret = NULL;
	xplQName single_qname;
	bool free_needed = false;

	if (cfgWarnOnNoExpectParam && cmd_params->expect == XPL_EXPECT_UNSPECIFIED)
		xplDisplayMessage(xplMsgWarning, BAD_CAST "no expect attribute in get-param command (file \"%s\", line %d)",
			commandInfo->element->doc->URL, commandInfo->element->line);

	if (cmd_params->name)
	{
		/* specified parameter */
		cmd_params->repeat &= cmd_params->show_tags || cmd_params->response_tag_name.ncname;
		values = xplParamGet(commandInfo->document->environment, cmd_params->name);
		if (values && !(values->type & cmd_params->type))
			values = NULL; /* skip unwanted */
		else if (!values && cmd_params->default_value) {
			values = xplParamValuesCreate();
			free_needed = true;
			xplParamValuesAdd(values, cmd_params->default_value, XPL_PARAM_TYPE_USERDATA);
		}
		if (!values)
			NOOP(); /* nothing to do */
		else if (cmd_params->show_tags || cmd_params->response_tag_name.ncname) { /* wrap in elements */
			if (cmd_params->show_tags)
				single_qname.ncname = cmd_params->name;
			ret = xplParamValuesToList(
				values,
				cmd_params->unique,
				cmd_params->expect,
				cmd_params->show_tags? single_qname: cmd_params->response_tag_name,
				commandInfo->element);
		} else { /* stringify */
			txt = xplParamValuesToString(values, cmd_params->unique, cmd_params->delimiter, cmd_params->expect);
			if (txt)
			{
				ret = xmlNewDocText(commandInfo->document->document, NULL);
				ret->content = txt;
			}
		}
	} else /* all parameters */
		ret = xplParamsToList(
			commandInfo->document->environment,
			cmd_params->unique,
			cmd_params->expect,
			cmd_params->show_tags? empty_qname: cmd_params->response_tag_name.ncname? cmd_params->response_tag_name: default_qname,
			commandInfo->element,
			cmd_params->type);
	ASSIGN_RESULT(ret, cmd_params->repeat, true);

	if (free_needed)
		xplParamValuesFree(values);
}
