#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsave.h>
#include <libxpl/xpltree.h>

void xplCmdSerializeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdSerializeParams
{
	xmlXPathObjectPtr select;
} xplCmdSerializeParams, *xplCmdSerializeParamsPtr;

static const xplCmdSerializeParams params_stencil =
{
	.select = NULL
};

xplCommand xplSerializeCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdSerializeEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdSerializeParams),
	.parameters = {
		{
			.name = BAD_CAST "select",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_NODESET,
			.value_stencil = &params_stencil.select
		}, {
			.name = NULL
		}
	}
};

void xplCmdSerializeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSerializeParamsPtr params = (xplCmdSerializeParamsPtr) commandInfo->params;
	xmlChar *txt;
	xmlNodePtr ret;

	if (params->select)
	{
		if (params->select->type != XPATH_NODESET)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath expression (%s) evaluated to non-nodeset value", params->select), true, true);
			return;
		}
		txt = serializeNodeSet(params->select->nodesetval);
	} else
		txt = serializeNodeList(commandInfo->element->children);
	ret = xmlNewDocText(commandInfo->element->doc, NULL);
	ret->content = txt;
	ASSIGN_RESULT(ret, false, true);
}
