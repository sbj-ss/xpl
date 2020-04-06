#include "commands/Inherit.h"
#include "Core.h"
#include "Messages.h"
#include "Options.h"
#include "Utils.h"

void xplCmdInheritPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdInheritEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")
#define SKIP_CURRENT_ATTR (BAD_CAST "skipcurrent")
	xmlChar *name_attr = NULL, *tagname;
	xmlNsPtr ns;
	xplMacroPtr macro = NULL;
	bool skip_current = true;
	xmlNodePtr ret, error, original_content = NULL, caller, temp_children;

	if (!commandInfo->document->current_macro)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "not inside macro definition or call"), true, true);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, SKIP_CURRENT_ATTR, &skip_current, skip_current)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	if (!name_attr)
	{
		tagname = commandInfo->document->current_macro->name;
		ns = commandInfo->document->current_macro->ns;
	} else {
		EXTRACT_NS_AND_TAGNAME(name_attr, ns, tagname, commandInfo->element);
	}
	macro = xplMacroLookup(commandInfo->element, ns? ns->href: NULL, tagname);
	if (skip_current && macro)
	{
		caller = macro->caller;
		original_content = macro->node_original_content;
		macro = xplMacroLookup(macro->parent->parent, ns? ns->href: NULL, tagname);
	} else
		caller = original_content = NULL;
	if (!macro && cfgWarnOnMissingInheritBase)
		xplDisplayMessage(xplMsgWarning, BAD_CAST "%s:%s: no macro \"%s:%s\" to inherit from", 
			commandInfo->element->ns? commandInfo->element->ns->prefix: NULL,
			commandInfo->element->name,
			ns? ns->prefix: NULL,
			tagname);
	if (macro && macro->content)
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
			ret = cloneNodeList(macro->content, commandInfo->element->parent, commandInfo->element->doc);
	} else
		ret = NULL;
	ASSIGN_RESULT(ret, true, true);
	if (name_attr) xmlFree(name_attr);
}

xplCommand xplInheritCommand = { xplCmdInheritPrologue, xplCmdInheritEpilogue }; 
