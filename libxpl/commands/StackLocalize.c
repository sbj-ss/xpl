#include <libxpl/xplcore.h>
#include <libxpl/xpltree.h>

void xplCmdStackLocalizePrologue(xplCommandInfoPtr commandInfo);
void xplCmdStackLocalizeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

void xplCmdStackLocalizePrologue(xplCommandInfoPtr commandInfo)
{
	commandInfo->prologue_state = commandInfo->document->stack;
	commandInfo->document->stack = NULL;
}

void xplCmdStackLocalizeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR (BAD_CAST "repeat")
	bool repeat;
	xmlNodePtr old_stack = (xmlNodePtr) commandInfo->prologue_state;
	xmlNodePtr error;

	xplClearDocStack(commandInfo->document);
	commandInfo->document->stack = old_stack;
	if (commandInfo->element->type & XPL_NODE_DELETION_MASK)
	{
		ASSIGN_RESULT(NULL, false, false);
	} else {
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, false)))
		{
			ASSIGN_RESULT(error, true, true);
			return;
		} else
			ASSIGN_RESULT(xplDetachContent(commandInfo->element), repeat, true);
	}
}

xplCommand xplStackLocalizeCommand = {
	.prologue = xplCmdStackLocalizePrologue, 
	.epilogue = xplCmdStackLocalizeEpilogue,
	.initializer = NULL,
	.finalizer = NULL,
	.flags = XPL_CMD_FLAG_CONTENT_SAFE
};
