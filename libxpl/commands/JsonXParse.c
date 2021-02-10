#include <libxpl/xplcommand.h>
#include <libxpl/xplcore.h>
#include <libxpl/xpljsonx.h>
#include <libxpl/xpltree.h>

void xplCmdJsonXParseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdJsonXParseParams
{
	bool validate;
	bool repeat;
} xplCmdJsonXParseParams, *xplCmdJsonXParseParamsPtr;

static const xplCmdJsonXParseParams params_stencil =
{
	.validate = true,
	.repeat = true
};

xplCommand xplJsonXParseCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdJsonXParseEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdJsonXParseParams),
	.parameters = {
		{
			.name = BAD_CAST "validate",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.validate
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};

void xplCmdJsonXParseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdJsonXParseParamsPtr params = (xplCmdJsonXParseParamsPtr) commandInfo->params;

	if (commandInfo->element->children)
		xplDocDeferNodeListDeletion(commandInfo->document, xplDetachChildren(commandInfo->element));
	ASSIGN_RESULT(xplJsonXParse(commandInfo->content, commandInfo->element, params->validate), params->repeat, true);
}
