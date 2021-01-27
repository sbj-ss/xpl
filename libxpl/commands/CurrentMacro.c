#include <libxpl/xplcore.h>
#include <libxpl/xplmacro.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdCurrentMacroEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdCurrentMacroParams
{
	xplQName tagname;
	bool repeat;
	bool detailed;
} xplCmdCurrentMacroParams, *xplCmdCurrentMacroParamsPtr;

static const xplCmdCurrentMacroParams params_stencil =
{
	.tagname = { NULL, BAD_CAST "macro" },
	.repeat = true,
	.detailed = false
};

xplCommand xplCurrentMacroCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdCurrentMacroEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdCurrentMacroParams),
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
			.name = BAD_CAST "detailed",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.detailed
		}, {
			.name = NULL
		}
	}
};

void xplCmdCurrentMacroEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdCurrentMacroParamsPtr params = (xplCmdCurrentMacroParamsPtr) commandInfo->params;
	xmlNodePtr ret;

	if (!commandInfo->document->current_macro)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "no active macros"), true, true);
		return;
	}
	if (params->detailed)
	{
		ASSIGN_RESULT(xplMacroToNode(commandInfo->document->current_macro, params->tagname, commandInfo->element), params->repeat, true);
		xplLiftNsDefs(commandInfo->element);
	} else {
		ret = xmlNewDocText(commandInfo->element->doc, NULL);
		ret->content = xplQNameToStr(commandInfo->document->current_macro->qname);
		ASSIGN_RESULT(ret, false, true);
	}
}
