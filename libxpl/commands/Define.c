#include <libxpl/xplcore.h>

void xplCmdDefinePrologue(xplCommandInfoPtr commandInfo);
void xplCmdDefineEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdDefineParams
{
	xplQName name;
	xmlChar *id;
	bool replace;
	xplMacroExpansionState expand;
} xplCmdDefineParams, *xplCmdDefineParamsPtr;

static const xplCmdDefineParams params_stencil =
{
	.name = { NULL, NULL },
	.id = NULL,
	.replace = true,
	.expand = XPL_MACRO_EXPAND_ALWAYS
};

xplCommand xplDefineCommand =
{
	.prologue = xplCmdDefinePrologue,
	.epilogue = xplCmdDefineEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_PROLOGUE | XPL_CMD_FLAG_HAS_CONTENT,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdDefineParams),
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.required = true,
			.value_stencil = &params_stencil.name
		}, {
			.name = BAD_CAST "id",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.id
		}, {
			.name = BAD_CAST "replace",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.replace
		}, {
			.name = BAD_CAST "expand",
			.type = XPL_CMD_PARAM_TYPE_INT_CUSTOM_GETTER,
			.extra.int_getter = xplMacroExpansionStateGetter,
			.value_stencil = &params_stencil.expand
		}, {
			.name = NULL
		}
	}
};

void xplCmdDefinePrologue(xplCommandInfoPtr commandInfo)
{
	xplCmdDefineParamsPtr params = (xplCmdDefineParamsPtr) commandInfo->params;

	xplAddMacro(commandInfo->document, commandInfo->element, params->name, commandInfo->element->parent, params->expand, params->replace, params->id);
}

void xplCmdDefineEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	UNUSED_PARAM(commandInfo);
	ASSIGN_RESULT(NULL, false, true);
}
