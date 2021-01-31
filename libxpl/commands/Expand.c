#include <libxpl/xplcore.h>
#include <libxpl/xpltree.h>

void xplCmdExpandPrologue(xplCommandInfoPtr commandInfo);
void xplCmdExpandEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplExpandCommand =
{
	.prologue = xplCmdExpandPrologue,
	.epilogue = xplCmdExpandEpilogue,
	.flags = XPL_CMD_FLAG_CONTENT_SAFE | XPL_CMD_FLAG_ALWAYS_EXPAND,
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
		ASSIGN_RESULT(xplDetachChildren(commandInfo->element), false, true);
	else
		ASSIGN_RESULT(NULL, false, true);
}
