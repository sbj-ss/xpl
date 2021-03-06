#include <libxpl/xplcommand.h>
#include <libxpl/xpljsonx.h>

void xplCmdJsonXSerializeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdJsonXSerializeParams
{
	bool strict_tag_names;
	bool value_type_check;
	bool format;
} xplCmdJsonXSerializeParams, *xplCmdJsonXSerializeParamsPtr;

static const xplCmdJsonXSerializeParams params_stencil =
{
	.strict_tag_names = false,
	.value_type_check = false,
	.format = false
};

xplCommand xplJsonXSerializeCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdJsonXSerializeEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdJsonXSerializeParams),
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
			.name = BAD_CAST "format",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.format
		}, {
			.name = NULL
		}
	}
};

void xplCmdJsonXSerializeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdJsonXSerializeParamsPtr params = (xplCmdJsonXSerializeParamsPtr) commandInfo->params;
	xmlNodePtr ret;

	ret = xplJsonXSerializeNodeList(commandInfo->element->children, params->strict_tag_names, params->value_type_check, params->format);
	ASSIGN_RESULT(ret, ret->type == XML_ELEMENT_NODE, true);
}
