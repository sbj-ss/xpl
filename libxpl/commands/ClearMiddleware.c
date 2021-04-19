#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplmiddleware.h>
#include <libxpl/xpltree.h>

void xplCmdClearMiddlewareEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplClearMiddlewareCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdClearMiddlewareEpilogue,
	.params_stencil = NULL,
	.stencil_size = 0,
	.flags = 0
};

void xplCmdClearMiddlewareEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (!xplDocSessionGetSaMode(commandInfo->document))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, "access denied"), true, true);
		return;
	}
	xplMWClear();
	ASSIGN_RESULT(NULL, false, true);
}
