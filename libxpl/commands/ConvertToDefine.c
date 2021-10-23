#include <libxpl/xplcore.h>
#include <libxpl/xplmacro.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>

void xplCmdConvertToDefineEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdConvertToDefineParams
{
	xplMacroExpansionState default_expand;
	bool default_replace;
} xplCmdConvertToDefineParams, *xplCmdConvertToDefineParamsPtr;

static const xplCmdConvertToDefineParams params_stencil =
{
	.default_expand = XPL_MACRO_EXPAND_ALWAYS,
	.default_replace = true
};

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
			.extra.int_getter = xplMacroExpansionStateGetter,
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
	xmlNodePtr cur, error, first_error = NULL, last_error = NULL;
	xmlChar *id_attr, *expand_attr, *replace_attr;
	xplMacroExpansionState expand;
	int replace;
	xplQName qname;

	cur = commandInfo->element->children;
	while (cur)
	{
		error = NULL;
		if (cur->type == XML_ELEMENT_NODE)
		{
			expand_attr = xmlGetNoNsProp(cur, BAD_CAST "expand");
			if (expand_attr)
			{
				expand = xplMacroExpansionStateFromString(expand_attr);
				if (expand == XPL_MACRO_EXPAND_UNKNOWN)
				{
					error = xplCreateErrorNode(commandInfo->element, "invalid expand value '%s'", expand_attr);
					APPEND_NODE_TO_LIST(first_error, last_error, error);
				}
				XPL_FREE(expand_attr);
				if (error)
					goto next;
			} else
				expand = params->default_expand;
			replace_attr = xmlGetNoNsProp(cur, BAD_CAST "replace");
			if (replace_attr)
			{
				replace = xplGetBooleanValue(replace_attr);
				if (replace == -1)
				{
					error = xplCreateErrorNode(commandInfo->element, "invalid replace value '%s'", replace_attr);
					APPEND_NODE_TO_LIST(first_error, last_error, error);
				}
				XPL_FREE(replace_attr);
				if (error)
					goto next;
			} else
				replace = params->default_replace;
			id_attr = xmlGetNoNsProp(cur, CONTENT_ID_ATTR);
			qname.ns = cur->ns;
			qname.ncname = BAD_CAST cur->name;
			xplAddMacro(commandInfo->document, cur, qname, commandInfo->element->parent, expand, replace, id_attr);
			if (id_attr)
				XPL_FREE(id_attr);
		}
next:
		cur = cur->next;
	}
	ASSIGN_RESULT(first_error, !!first_error, true);
}
