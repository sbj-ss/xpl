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
	xmlNodePtr cur, error;
	xmlChar *id_attr, *expand_attr, *replace_attr;
	xplMacroExpansionState expand;
	int replace;
	xplQName qname;

	cur = commandInfo->element->children;
	while (cur)
	{
		if (cur->type == XML_ELEMENT_NODE)
		{
			expand_attr = xmlGetNoNsProp(cur, BAD_CAST "expand");
			if (expand_attr)
			{
				expand = xplMacroExpansionStateFromString(expand_attr);
				XPL_FREE(expand_attr);
				if (expand == XPL_MACRO_EXPAND_UNKNOWN)
				{
					error = xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid expand value '%s'", expand_attr);
					xplReplaceWithList(cur, error);
					cur = error->next;
					continue;
				}
			} else
				expand = params->default_expand;
			replace_attr = xmlGetNoNsProp(cur, BAD_CAST "replace");
			if (replace_attr)
			{
				replace = xplGetBooleanValue(replace_attr);
				XPL_FREE(replace_attr);
				if (replace == -1)
				{
					error = xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid replace value '%s'", replace_attr);
					xplReplaceWithList(cur, error);
					cur = error->next;
					continue;
				}
			} else
				replace = params->default_replace;
			id_attr = xmlGetNoNsProp(cur, BAD_CAST "id");
			qname.ns = cur->ns;
			qname.ncname = BAD_CAST cur->name;
			xplAddMacro(commandInfo->document, cur, qname, commandInfo->element->parent, expand, replace, id_attr);
			if (id_attr)
				XPL_FREE(id_attr);
		}
		cur = cur->next;
	}
	ASSIGN_RESULT(NULL, false, true);
}
