#include <libxpl/xplcore.h>
#include <libxpl/xpltree.h>

void xplCmdExpandAfterPrologue(xplCommandInfoPtr commandInfo);
void xplCmdExpandAfterEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);
void xplCmdExpandAfterRestoreState(xplCommandInfoPtr commandInfo);

xplCommand xplExpandAfterCommand =
{
	.prologue = xplCmdExpandAfterPrologue,
	.epilogue = xplCmdExpandAfterEpilogue,
	.restore_state = xplCmdExpandAfterRestoreState,
	.flags = XPL_CMD_FLAG_ALWAYS_EXPAND,
	.params_stencil = NULL
};

void xplCmdExpandAfterPrologue(xplCommandInfoPtr commandInfo)
{
	commandInfo->prologue_state = (void*) commandInfo->document->expand;
	commandInfo->document->expand = false;
}

void xplCmdExpandAfterEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	ASSIGN_RESULT(xplDetachChildren(commandInfo->element), true, true);
}

void xplCmdExpandAfterRestoreState(xplCommandInfoPtr commandInfo)
{
	commandInfo->document->expand = (bool) commandInfo->prologue_state;
}
