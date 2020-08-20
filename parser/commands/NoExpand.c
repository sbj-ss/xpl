#include <libxpl/xplcore.h>
#include <libxpl/xpltree.h>
#include "commands/NoExpand.h"

xplCommand xplNoExpandCommand =
{
	.prologue = xplCmdNoExpandPrologue,
	.epilogue = xplCmdNoExpandEpilogue,
	.flags = XPL_CMD_FLAG_CONTENT_SAFE,
	.parameters = {
		{ .name = NULL }
	}
};

void xplCmdNoExpandPrologue(xplCommandInfoPtr commandInfo)
{
	commandInfo->prologue_state = (void*) commandInfo->document->expand;
	commandInfo->document->expand = false;
}

void xplCmdNoExpandEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	commandInfo->document->expand = (bool) commandInfo->prologue_state;
	if (!(commandInfo->element->type & XPL_NODE_DELETION_MASK))
		ASSIGN_RESULT(xplDetachContent(commandInfo->element), true, true);
	else
		ASSIGN_RESULT(NULL, false, true);
}
