#include <libxpl/xplcore.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdOtherwiseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdOtherwiseParams
{
	bool repeat;
} xplCmdOtherwiseParams, *xplCmdOtherwiseParamsPtr;

static const xplCmdOtherwiseParams params_stencil =
{
	.repeat = false
};

xplCommand xplOtherwiseCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdOtherwiseEpilogue,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdOtherwiseParams),
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.parameters = {
		{
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};

void xplCmdOtherwiseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdOtherwiseParamsPtr params = (xplCmdOtherwiseParamsPtr) commandInfo->params;
	xmlNodePtr brk;
	xmlNsPtr brk_ns;

	ASSIGN_RESULT(xplDetachChildren(commandInfo->element), params->repeat, true);
	brk_ns = commandInfo->document->root_xpl_ns;
	if (!brk_ns)
		brk_ns = xmlSearchNsByHref(commandInfo->element->doc, commandInfo->element, cfgXplNsUri);
	brk = xmlNewDocNode(commandInfo->element->doc, brk_ns, BAD_CAST "break", NULL);
	xplAppendList(commandInfo->element, brk);
}
