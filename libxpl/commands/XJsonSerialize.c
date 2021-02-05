#include <libxpl/xplbuffer.h>
#include <libxpl/xplcommand.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>
#include <libxpl/xplxjson.h>

void xplCmdXJsonSerializeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdXJsonSerializeParams
{
	bool strict_tag_names;
	bool value_type_check;
} xplCmdXJsonSerializeParams, *xplCmdXJsonSerializeParamsPtr;

static const xplCmdXJsonSerializeParams params_stencil =
{
	.strict_tag_names = false,
	.value_type_check = false,
};

xplCommand xplXJsonSerializeCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdXJsonSerializeEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdXJsonSerializeParams),
	.parameters = {
		{
			.name = BAD_CAST "stricttagnames",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.strict_tag_names
		}, {
			.name = BAD_CAST "valuetypecheck",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.value_type_check
		}, {
			.name = NULL
		}
	}
};

void xplCmdXJsonSerializeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdXJsonSerializeParamsPtr params = (xplCmdXJsonSerializeParamsPtr) commandInfo->params;
	xmlNodePtr ret;

	ret = xplXJsonSerializeNodeList(commandInfo->element->children, params->strict_tag_names, params->value_type_check);
	ASSIGN_RESULT(ret, ret->type == XML_ELEMENT_NODE, true);
}
