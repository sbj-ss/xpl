#include <libxpl/xplcore.h>
#include <libxpl/xpltree.h>

void xplCmdNoExpandPrologue(xplCommandInfoPtr commandInfo);
void xplCmdNoExpandEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);
void xplCmdNoExpandRestoreState(xplCommandInfoPtr commandInfo);

xplCommand xplNoExpandCommand =
{
	.prologue = xplCmdNoExpandPrologue,
	.epilogue = xplCmdNoExpandEpilogue,
	.restore_state = xplCmdNoExpandRestoreState,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdNoExpandPrologue(xplCommandInfoPtr commandInfo)
{
	commandInfo->prologue_state = (void*) commandInfo->document->expand;
	commandInfo->document->expand = false;
}

void xplCmdNoExpandEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	ASSIGN_RESULT(xplDetachChildren(commandInfo->element), false, true);
}

void xplCmdNoExpandRestoreState(xplCommandInfoPtr commandInfo)
{
	commandInfo->document->expand = (bool) commandInfo->prologue_state;
}
