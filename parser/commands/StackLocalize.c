#include <libxpl/xplcore.h>
#include <libxpl/xplutils.h>
#include "commands/StackLocalize.h"

void xplCmdStackLocalizePrologue(xplCommandInfoPtr commandInfo)
{
	commandInfo->_private = commandInfo->document->stack;
	commandInfo->document->stack = NULL;
}

void xplCmdStackLocalizeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR (BAD_CAST "repeat")
	bool repeat;
	xmlNodePtr old_stack = (xmlNodePtr) commandInfo->_private;
	xmlNodePtr error;

	xplClearDocStack(commandInfo->document);
	commandInfo->document->stack = old_stack;
	if (commandInfo->element->type & XML_NODE_DELETION_MASK)
	{
		ASSIGN_RESULT(NULL, false, false);
	} else {
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, false)))
		{
			ASSIGN_RESULT(error, true, true);
			return;
		} else
			ASSIGN_RESULT(detachContent(commandInfo->element), repeat, true);
	}
}

xplCommand xplStackLocalizeCommand = {
	SFINIT(.prologue, xplCmdStackLocalizePrologue), 
	SFINIT(.epilogue, xplCmdStackLocalizeEpilogue),
	SFINIT(.initializer, NULL),
	SFINIT(.finalizer, NULL),
	SFINIT(.flags, XPL_CMD_FLAG_CONTENT_SAFE)
};
