#include <libxpl/xplcore.h>
#include <libxpl/xplmacro.h>
#include <libxpl/xplmessages.h>
#include "commands/ConvertToDefine.h"

typedef struct _xplCmdConvertToDefineParams
{
	xplMacroExpansionState default_expand;
	bool default_replace;
} xplCmdConvertToDefineParams, *xplCmdConvertToDefineParamsPtr;

static const xplCmdConvertToDefineParams params_stencil =
{
	.default_expand = XPL_MACRO_EXPAND_NO_DEFAULT,
	.default_replace = true
};

static xmlChar* _getDefaultExpand(xplCommandInfoPtr info, const xmlChar *raw_value, int *result)
{
	UNUSED_PARAM(info);
	*result = xplMacroExpansionStateFromString(raw_value, true);
	if (*result == XPL_MACRO_EXPAND_UNKNOWN)
		return xplFormatMessage(BAD_CAST "invalid defaultexpand value '%s'", raw_value);
	return NULL;
}

xplCommand xplConvertToDefineCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdConvertToDefineEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdConvertToDefineParams),
	.parameters = {
		{
			.name = BAD_CAST "defaultexpand",
			.type = XPL_CMD_PARAM_TYPE_INT_CUSTOM_GETTER,
			.extra = {
				.int_getter = _getDefaultExpand
			},
			.value_stencil = &params_stencil.default_expand
		}, {
			.name = BAD_CAST "defaultreplace",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.default_replace
		}, {
			.name = NULL
		}
	}
};

void xplCmdConvertToDefineEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdConvertToDefineParamsPtr params = (xplCmdConvertToDefineParamsPtr) commandInfo->params;
	xmlNodePtr tmp;

	tmp = commandInfo->element->children;
	while (tmp)
	{
		if (tmp->type == XML_ELEMENT_NODE)
			xplAddMacro(commandInfo->document, tmp, commandInfo->element->parent, true, params->default_expand, params->default_replace);
		tmp = tmp->next;
	}
	ASSIGN_RESULT(NULL, false, true);
}
