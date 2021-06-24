#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplparams.h>
#include <libxpl/xpltree.h>

void xplCmdGetParamEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdGetParamParams
{
	xmlChar *default_value;
	xmlChar *delimiter;
	xplExpectType expect;
	xmlChar *name;
	bool repeat;
	xplQName tag_name;
	bool show_tags;
	int type;
	bool unique;
} xplCmdGetParamParams, *xplCmdGetParamParamsPtr;

static const xplCmdGetParamParams params_stencil =
{
	.default_value = NULL,
	.delimiter = NULL,
	.expect = XPL_EXPECT_UNSPECIFIED,
	.name = NULL,
	.repeat = true,
	.tag_name = {},
	.show_tags = false,
	.type = XPL_PARAM_TYPE_USERDATA,
	.unique = false
};

static xmlChar* tagname_aliases[] = { BAD_CAST "responsetagname", NULL };

xplCommand xplGetParamCommand =
{
	.prologue = NULL,
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
			.name = BAD_CAST "tagname",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.aliases = tagname_aliases,
			.value_stencil = &params_stencil.tag_name
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

static const xplQName default_qname = { NULL, BAD_CAST "param" };
static const xplQName empty_qname = { NULL, NULL };

void xplCmdGetParamEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdGetParamParamsPtr params = (xplCmdGetParamParamsPtr) commandInfo->params;
	xmlChar *txt;
	xplParamValuesPtr values;
	xmlNodePtr ret = NULL;
	xplQName single_qname;
	bool free_needed = false;

	if (cfgWarnOnNoExpectParam && params->expect == XPL_EXPECT_UNSPECIFIED)
		xplDisplayWarning(commandInfo->element, "no expect attribute");

	if (params->show_tags && params->tag_name.ncname)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "showtags and tagname can't be used simultaneously"), true, true);
		return;
	}
	if ((params->show_tags || params->tag_name.ncname) && params->delimiter)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "delimiter can't be used simultaneously with showtags or tagname"), true, true);
		return;
	}
	if (params->default_value && !params->name)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "default value can be specified only for a single parameter"), true, true);
		return;
	}

	if (params->name)
	{
		/* specified parameter */
		params->repeat &= params->show_tags || params->tag_name.ncname;
		values = xplParamGet(commandInfo->document->params, params->name);
		if (values && !(values->type & params->type))
			values = NULL; /* skip unwanted */
		else if (!values && params->default_value) {
			values = xplParamValuesCreate();
			free_needed = true;
			xplParamValuesAdd(values, params->default_value, XPL_PARAM_TYPE_USERDATA);
			params->default_value = NULL;
		}
		if (!values)
			NOOP(); /* nothing to do */
		else if (params->show_tags || params->tag_name.ncname) { /* wrap in elements */
			if (params->show_tags)
			{
				single_qname.ncname = params->name;
				single_qname.ns = NULL;
			}
			ret = xplParamValuesToList(
				values,
				params->unique,
				params->expect,
				params->show_tags? single_qname: params->tag_name,
				commandInfo->element);
		} else { /* stringify */
			txt = xplParamValuesToString(values, params->unique, params->delimiter, params->expect);
			if (!txt)
			{
				params->repeat = true;
				ret = xplCreateErrorNode(commandInfo->element, "some param types can't be represented as string");
			} else {
				ret = xmlNewDocText(commandInfo->document->document, NULL);
				ret->content = txt;
			}
		}
	} else /* all parameters */
		ret = xplParamsToList(
			commandInfo->document->params,
			params->unique,
			params->expect,
			params->show_tags? empty_qname: params->tag_name.ncname? params->tag_name: default_qname,
			commandInfo->element,
			params->type);
	ASSIGN_RESULT(ret, params->repeat, true);

	if (free_needed)
		xplParamValuesFree(values);
}
