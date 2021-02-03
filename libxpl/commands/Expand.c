#include <libxpl/xplcore.h>
#include <libxpl/xpltree.h>

void xplCmdExpandPrologue(xplCommandInfoPtr commandInfo);
void xplCmdExpandEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);
void xplCmdExpandRestoreState(xplCommandInfoPtr commandInfo);

xplCommand xplExpandCommand =
{
	.prologue = xplCmdExpandPrologue,
	.epilogue = xplCmdExpandEpilogue,
	.restore_state = xplCmdExpandRestoreState,
	.flags = XPL_CMD_FLAG_ALWAYS_EXPAND,
	.params_stencil = NULL
};

void xplCmdExpandPrologue(xplCommandInfoPtr commandInfo)
{
	commandInfo->prologue_state = (void*) commandInfo->document->expand;
	commandInfo->document->expand = true;
}

void xplCmdExpandEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	ASSIGN_RESULT(xplDetachChildren(commandInfo->element), false, true);
}

void xplCmdExpandRestoreState(xplCommandInfoPtr commandInfo)
{
	commandInfo->document->expand = (bool) commandInfo->prologue_state;
}
