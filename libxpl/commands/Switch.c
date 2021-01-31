#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdSwitchPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSwitchEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdSwitchParams
{
	xmlXPathObjectPtr key;
	bool repeat;
} xplCmdSwitchParams, *xplCmdSwitchParamsPtr;

static const xplCmdSwitchParams params_stencil =
{
	.key = NULL,
	.repeat = false
};

xplCommand xplSwitchCommand = {
	.prologue = xplCmdSwitchPrologue,
	.epilogue = xplCmdSwitchEpilogue,
	.flags = XPL_CMD_FLAG_CONTENT_SAFE | XPL_CMD_FLAG_PARAMS_FOR_PROLOGUE | XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdSwitchParams),
	.parameters = {
		{
			.name = BAD_CAST "key",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.required = true,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_ANY,
			.value_stencil = &params_stencil.key
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};


void xplCmdSwitchPrologue(xplCommandInfoPtr commandInfo)
{
	xplCmdSwitchParamsPtr params = (xplCmdSwitchParamsPtr) commandInfo->params;

	commandInfo->element->content = (xmlChar*) params->key;
}

void xplCmdSwitchEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSwitchParamsPtr params = (xplCmdSwitchParamsPtr) commandInfo->params;
	xmlXPathObjectPtr nodes = (xmlXPathObjectPtr) commandInfo->element->content;

	if (commandInfo->element->type & XPL_NODE_DELETION_MASK)
	{
		if (nodes->nodesetval)
			nodes->nodesetval->nodeNr = 0;
		ASSIGN_RESULT(NULL, false, false);
	} else if (nodes)
		ASSIGN_RESULT(xplDetachChildren(commandInfo->element), params->repeat, true);
}
