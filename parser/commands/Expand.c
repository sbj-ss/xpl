#include <libxpl/xplcore.h>
#include <libxpl/xpltree.h>
#include "commands/Expand.h"

xplCommand xplExpandCommand =
{
	.prologue = xplCmdExpandPrologue,
	.epilogue = xplCmdExpandEpilogue,
	.flags = XPL_CMD_FLAG_CONTENT_SAFE,
	.params_stencil = NULL
};

void xplCmdExpandPrologue(xplCommandInfoPtr commandInfo)
{
	commandInfo->prologue_state = (void*) commandInfo->document->expand;
	commandInfo->document->expand = true;
}

void xplCmdExpandEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	commandInfo->document->expand = (bool) commandInfo->prologue_state;
	if (!(commandInfo->element->type & XPL_NODE_DELETION_MASK))
		ASSIGN_RESULT(xplDetachContent(commandInfo->element), true, true);
	else
		ASSIGN_RESULT(NULL, false, true);
}
