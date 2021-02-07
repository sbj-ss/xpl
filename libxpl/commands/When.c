#include <libxpl/xplcore.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdWhenPrologue(xplCommandInfoPtr commandInfo);
void xplCmdWhenEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdWhenParams
{
	xmlChar *test;
	bool repeat;
} xplCmdWhenParams, *xplCmdWhenParamsPtr;

static const xplCmdWhenParams params_stencil =
{
	.test = NULL,
	.repeat = false
};

xplCommand xplWhenCommand =
{
	.prologue = xplCmdWhenPrologue,
	.epilogue = xplCmdWhenEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_PROLOGUE | XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdWhenParams),
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

void xplCmdWhenPrologue(xplCommandInfoPtr commandInfo)
{
	xplCmdWhenParamsPtr params = (xplCmdWhenParamsPtr) commandInfo->params;
	xmlNsPtr xpl_ns;
	xmlNodePtr test_el;

	if (params->test)
	{
		if (!(xpl_ns = commandInfo->document->root_xpl_ns))
			xpl_ns = xmlSearchNsByHref(commandInfo->element->doc, commandInfo->element, cfgXplNsUri);
		test_el = xmlNewDocNode(commandInfo->element->doc, xpl_ns, BAD_CAST "test", NULL);
		test_el->children = test_el->last = xmlNewDocText(commandInfo->element->doc, NULL);
		test_el->children->parent = test_el;
		test_el->children->content = params->test;
		params->test = NULL;
		xplPrependChildren(commandInfo->element, test_el);
	}
}

void xplCmdWhenEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	/* ToDo: :break should be added here and not by :test
	 * motivation: :choose implies single choice but :when without condition will always succeed.
     */
	xplCmdWhenParamsPtr params = (xplCmdWhenParamsPtr) commandInfo->params;

	ASSIGN_RESULT(xplDetachChildren(commandInfo->element), params->repeat, true);
}
