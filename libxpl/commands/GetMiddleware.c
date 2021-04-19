#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplmiddleware.h>
#include <libxpl/xpltree.h>

void xplCmdGetMiddlewareEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdGetMiddlewareParams
{
	xmlChar *regex;
	xplQName tag_name;
	bool repeat;
} xplCmdGetMiddlewareParams, *xplCmdGetMiddlewareParamsPtr;

static const xplCmdGetMiddlewareParams params_stencil =
{
	.regex = NULL,
	.tag_name = { NULL, NULL },
	.repeat = true
};

static xmlChar* tagname_aliases[] = { BAD_CAST "responsetagname", NULL };

xplCommand xplGetMiddlewareCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdGetMiddlewareEpilogue,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdGetMiddlewareParams),
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.parameters = {
		{
			.name = BAD_CAST "regex",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.regex
		}, {
			.name = BAD_CAST "tagname",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.aliases = tagname_aliases,
			.value_stencil = &params_stencil.tag_name
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};

static const xplQName default_qname = { NULL, BAD_CAST "middleware" };

void xplCmdGetMiddlewareEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdGetMiddlewareParamsPtr params = (xplCmdGetMiddlewareParamsPtr) commandInfo->params;
	const xmlChar *file;
	xmlNodePtr ret;
	xplQName tagname;

	if (!xplDocSessionGetSaMode(commandInfo->document))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "access denied"), true, true);
		return;
	}

	if (params->regex)
	{
		file = xplMWGetWrapper(params->regex);
		if (file)
			ret = xmlNewDocText(commandInfo->element->doc, file);
		else
			ret = NULL;
		params->repeat = false;
	} else {
		tagname = params->tag_name.ncname? params->tag_name: default_qname;
		ret = xplMWListEntries(commandInfo->document->document, tagname);
	}
	ASSIGN_RESULT(ret, params->repeat, true);
}
