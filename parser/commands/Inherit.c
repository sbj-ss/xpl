#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>
#include "commands/Inherit.h"

typedef struct _xplCmdInheritParams
{
	xplQName name;
	bool skip_current;
	bool repeat;
} xplCmdInheritParams, *xplCmdInheritParamsPtr;

static const xplCmdInheritParams params_stencil =
{
	.name = { NULL, NULL },
	.skip_current = true,
	.repeat = false
};

xplCommand xplInheritCommand =
{
	.prologue = xplCmdInheritPrologue,
	.epilogue = xplCmdInheritEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdInheritParams),
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.value_stencil = &params_stencil.name
		}, {
			.name = BAD_CAST "skipcurrent",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.skip_current
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};

static inline xplMacroPtr _macroLookup(xmlNodePtr node, xplQName name)
{
	return xplMacroLookup(node, name.ns? name.ns->href: NULL, name.ncname);
}

void xplCmdInheritPrologue(xplCommandInfoPtr commandInfo)
{
	UNUSED_PARAM(commandInfo);
}

void xplCmdInheritEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdInheritParamsPtr params = (xplCmdInheritParamsPtr) commandInfo->params;
	xplQName macro_name;
	xplMacroPtr macro = NULL;
	bool skip_current = true;
	xmlNodePtr ret, original_content = NULL, caller, temp_children;

	if (!commandInfo->document->current_macro)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "not inside macro definition or call"), true, true);
		return;
	}
	if (params->name.ncname)
		macro_name = params->name;
	else {
		macro_name.ncname = commandInfo->document->current_macro->name;
		macro_name.ns = commandInfo->document->current_macro->ns;
	}
	macro = _macroLookup(commandInfo->element, macro_name);
	if (skip_current && macro)
	{
		caller = macro->caller;
		original_content = macro->node_original_content;
		macro = _macroLookup(macro->parent->parent, macro_name);
	} else
		caller = original_content = NULL;
	if (!macro)
	{
		if (cfgWarnOnMissingInheritBase)
			xplDisplayMessage(xplMsgWarning, BAD_CAST "%s:%s: no macro \"%s:%s\" to inherit from",
				commandInfo->element->ns? commandInfo->element->ns->prefix: NULL,
				commandInfo->element->name,
				macro_name.ns? macro_name.ns->prefix: NULL,
				macro_name.ncname);
		ASSIGN_RESULT(NULL, false, true);
		return;
	}
	if (macro->content)
	{
		if (((macro->expansion_state == XPL_MACRO_EXPAND_ALWAYS) || (macro->expansion_state == XPL_MACRO_EXPAND_ONCE)) && caller)
		{
			/* restore original content temporarily */
			temp_children = caller->children;
			caller->children = original_content;
			while (original_content->next)
				original_content = original_content->next;
			caller->last = original_content;
			ret = xplReplaceContentEntries(commandInfo->document, macro->id, caller, macro->content);
			/* restore children */
			caller->children = temp_children;
			while (temp_children->next)
				temp_children = temp_children->next;
			caller->last = temp_children;
		} else
			ret = xplCloneNodeList(macro->content, commandInfo->element->parent, commandInfo->element->doc);
	} else
		ret = NULL;
	ASSIGN_RESULT(ret, params->repeat, true);
}
