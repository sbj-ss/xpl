#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdInheritEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

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
	.prologue = NULL,
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
	else
		macro_name = commandInfo->document->current_macro->qname;
	macro = xplMacroLookupByQName(commandInfo->element, macro_name);
	if (skip_current && macro)
	{
		caller = macro->caller;
		original_content = macro->node_original_content;
		macro = xplMacroLookupByQName(macro->parent->parent, macro_name);
	} else
		caller = original_content = NULL;
	if (!macro)
	{
		if (cfgWarnOnMissingInheritBase)
			xplDisplayWarning(commandInfo->element, BAD_CAST "no macro '%s:%s' to inherit from", macro_name.ns? macro_name.ns->prefix: NULL, macro_name.ncname);
		ASSIGN_RESULT(NULL, false, true);
		return;
	}
	if (macro->content)
	{
		if (((macro->expansion_state == XPL_MACRO_EXPAND_ALWAYS) || (macro->expansion_state == XPL_MACRO_EXPAND_ONCE)) && caller)
		{
			temp_children = caller->children;
			xplSetChildren(caller, original_content);
			ret = xplReplaceContentEntries(commandInfo->document, macro->id, caller, macro->content, commandInfo->element);
			xplSetChildren(caller, temp_children); // TODO сюда попадает сама команда
		} else
			ret = xplCloneNodeList(macro->content, commandInfo->element->parent, commandInfo->element->doc);
	} else
		ret = NULL;
	ASSIGN_RESULT(ret, params->repeat, true);
}
