#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>
#include "commands/Default.h"

void xplCmdDefaultPrologue(xplCommandInfoPtr commandInfo)
{
	xmlNodePtr parent = commandInfo->element->parent;
	if (!parent->ns || 
		((parent->ns != commandInfo->document->root_xpl_ns) && xmlStrcmp(parent->ns->href, cfgXplNsUri)) ||
		xmlStrcmp(parent->name, BAD_CAST "switch"))
	{
		commandInfo->_private = xplCreateErrorNode(commandInfo->element, BAD_CAST "parent tag must be :switch");
		xplDocDeferNodeListDeletion(commandInfo->document, xplDetachContent(commandInfo->element));
	}
}

void xplCmdDefaultEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR (BAD_CAST "repeat")
#define BREAK_ATTR (BAD_CAST "break")
	bool repeat;
	bool do_break;
	xmlNodePtr error;

	if (commandInfo->_private)
	{
		ASSIGN_RESULT((xmlNodePtr) commandInfo->_private, true, true);
	} else {
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, false)))
		{
			ASSIGN_RESULT(error, true, true);
			return;
		}
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, BREAK_ATTR, &do_break, true)))
		{
			ASSIGN_RESULT(error, true, true);
		} else {
			ASSIGN_RESULT(xplDetachContent(commandInfo->element), repeat, true);
		}
		if (do_break)
		{
			xplDocDeferNodeListDeletion(commandInfo->document, commandInfo->element->next);
			commandInfo->element->next = NULL;
			commandInfo->element->parent->last = commandInfo->element;
		}
	}
}

xplCommand xplDefaultCommand = { xplCmdDefaultPrologue, xplCmdDefaultEpilogue };
