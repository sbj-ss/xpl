#include "commands/Default.h"
#include "Core.h"
#include "Messages.h"
#include "Options.h"
#include "Utils.h"

void xplCmdDefaultPrologue(xplCommandInfoPtr commandInfo)
{
	xmlNodePtr parent = commandInfo->element->parent;
	if (!parent->ns || 
		((parent->ns != commandInfo->document->root_xpl_ns) && xmlStrcmp(parent->ns->href, cfgXplNsUri)) ||
		xmlStrcmp(parent->name, BAD_CAST "switch"))
	{
		commandInfo->_private = xplCreateErrorNode(commandInfo->element, BAD_CAST "parent tag must be :switch");
		xplDocDeferNodeListDeletion(commandInfo->document, detachContent(commandInfo->element));
	}
}

void xplCmdDefaultEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR (BAD_CAST "repeat")
#define BREAK_ATTR (BAD_CAST "break")
	BOOL repeat;
	BOOL do_break;
	xmlNodePtr error;

	if (commandInfo->_private)
	{
		ASSIGN_RESULT((xmlNodePtr) commandInfo->_private, TRUE, TRUE);
	} else {
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, FALSE)))
		{
			ASSIGN_RESULT(error, TRUE, TRUE);
			return;
		}
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, BREAK_ATTR, &do_break, TRUE)))
		{
			ASSIGN_RESULT(error, TRUE, TRUE);
		} else {
			ASSIGN_RESULT(detachContent(commandInfo->element), repeat, TRUE);
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
