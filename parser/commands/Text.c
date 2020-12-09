#include <libxpl/xplcore.h>
#include <libxpl/xpltree.h>
#include "commands/Text.h"

void xplCmdTextEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR (BAD_CAST "repeat")

	bool repeat;
	xmlNodePtr error;

	commandInfo->document->indent_spinlock--;
	if (!(commandInfo->element->type & XPL_NODE_DELETION_MASK))
	{
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, false)))
		{
			ASSIGN_RESULT(error, true, true);
			return;
		} else
			ASSIGN_RESULT(xplDetachContent(commandInfo->element), repeat, true);
	} else
		ASSIGN_RESULT(NULL, false, false);
}

xplCommand xplTextCommand = {
	.prologue = NULL,
	.epilogue = xplCmdTextEpilogue,
	.initializer = NULL,
	.finalizer = NULL,
	.flags = XPL_CMD_FLAG_CONTENT_SAFE
};
