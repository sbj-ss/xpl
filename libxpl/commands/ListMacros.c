#include <libxpl/xplcommand.h>
#include <libxpl/xplmacro.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdListMacrosEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdListMacrosParams
{
	xplQName tagname;
	bool repeat;
	xmlChar *delimiter;
	bool unique;
} xplCmdListMacrosParams, *xplCmdListMacrosParamsPtr;

static const xplCmdListMacrosParams params_stencil =
{
	.tagname = { NULL, BAD_CAST "macro" },
	.repeat = true,
	.delimiter = NULL,
	.unique = false
};

xplCommand xplListMacrosCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdListMacrosEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdListMacrosParams),
	.parameters = {
		{
			.name = BAD_CAST "tagname",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.value_stencil = &params_stencil.tagname
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = BAD_CAST "delimiter",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.delimiter
		}, {
			.name = BAD_CAST "unique",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.unique
		}, {
			.name = NULL
		}
	}
};

void xplCmdListMacrosEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdListMacrosParamsPtr params = (xplCmdListMacrosParamsPtr) commandInfo->params;
	xmlNodePtr ret;

	if (params->delimiter && params->tagname.ncname)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "cannot use delimiter and tagname at the same time"), true, true);
		return;
	}
	if (params->delimiter)
	{
		ret = xmlNewDocText(commandInfo->element->doc, NULL);
		ret->content = xplMacroTableToString(commandInfo->element, params->delimiter, params->unique);
		ASSIGN_RESULT(ret, false, true);
	} else {
		ret = xplMacroTableToNodeList(commandInfo->element, params->tagname, params->unique, commandInfo->element);
		xplDownshiftNodeListNsDef(ret, commandInfo->element->nsDef);
		ASSIGN_RESULT(ret, params->repeat, true);
	}
}
