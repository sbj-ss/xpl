#include <libxpl/xplcore.h>
#include <libxpl/xpldb.h>

void xplCmdDbGarbageCollectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplDbGarbageCollectCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdDbGarbageCollectEpilogue,
	.flags = 0,
	.params_stencil = NULL,
	.stencil_size = 0
};

void xplCmdDbGarbageCollectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	UNUSED_PARAM(commandInfo);
	xplLockThreads(true);
	xplDbGarbageCollect();
	xplLockThreads(false);
	ASSIGN_RESULT(NULL, false, true);
}
