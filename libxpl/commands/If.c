#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdIfPrologue(xplCommandInfoPtr commandInfo);
void xplCmdIfEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdIfParams
{
	xmlChar *test;
	bool repeat;
} xplCmdIfParams, *xplCmdIfParamsPtr;

static const xplCmdIfParams params_stencil =
{
	.test = NULL,
	.repeat = false
};

xplCommand xplIfCommand =
{
	.prologue = xplCmdIfPrologue,
	.epilogue = xplCmdIfEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_PROLOGUE | XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdIfParams),
	.parameters = {
		{
			.name = BAD_CAST "test",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.test
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};

void xplCmdIfPrologue(xplCommandInfoPtr commandInfo)
{
	xplCmdIfParamsPtr params = (xplCmdIfParamsPtr) commandInfo->params;
	xmlNsPtr xplns;
	xmlNodePtr test_el;

	if (params->test)
	{
		if (!(xplns = commandInfo->document->root_xpl_ns))
			xplns = commandInfo->element->ns;
		test_el = xmlNewDocNode(commandInfo->element->doc, xplns, BAD_CAST "test", params->test);
		if (commandInfo->element->children)
			xplPrependList(commandInfo->element->children, test_el);
		else {
			commandInfo->element->children = commandInfo->element->last = test_el;
			test_el->parent = commandInfo->element;
		}
	}
}

void xplCmdIfEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdIfParamsPtr params = (xplCmdIfParamsPtr) commandInfo->params;
	
	ASSIGN_RESULT(xplDetachContent(commandInfo->element), params->repeat, true);
}
