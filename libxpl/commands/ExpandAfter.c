#include <libxpl/xplcore.h>
#include <libxpl/xpltree.h>

void xplCmdExpandAfterPrologue(xplCommandInfoPtr commandInfo);
void xplCmdExpandAfterEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplExpandAfterCommand =
{
	.prologue = xplCmdExpandAfterPrologue,
	.epilogue = xplCmdExpandAfterEpilogue,
	.flags = XPL_CMD_FLAG_CONTENT_SAFE | XPL_CMD_FLAG_ALWAYS_EXPAND,
	.params_stencil = NULL
};

void xplCmdExpandAfterPrologue(xplCommandInfoPtr commandInfo)
{
	commandInfo->prologue_state = (void*) commandInfo->document->expand;
	commandInfo->document->expand = false;
}

void xplCmdExpandAfterEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	commandInfo->document->expand = (bool) commandInfo->prologue_state;
	if (!(commandInfo->element->type & XPL_NODE_DELETION_MASK))
		ASSIGN_RESULT(xplDetachChildren(commandInfo->element), true, true);
	else
		ASSIGN_RESULT(NULL, false, true);
}
