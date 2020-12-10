#include <libxpl/xplcommand.h>
#include <stdio.h>

void xplCmdGetThreadIdEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplGetThreadIdCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdGetThreadIdEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdGetThreadIdEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	char buf[20];
	xmlNodePtr ret;

	snprintf(buf, sizeof(buf), XPR_THREAD_ID_FORMAT, xprGetCurrentThreadId());
	ret = xmlNewDocText(commandInfo->element->doc, BAD_CAST buf);
	ASSIGN_RESULT(ret, false, true);
}
