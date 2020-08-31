#include <stdio.h>
#include "commands/GetThreadId.h"

xplCommand xplGetThreadIdCommand =
{
	.prologue = xplCmdGetThreadIdPrologue,
	.epilogue = xplCmdGetThreadIdEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdGetThreadIdPrologue(xplCommandInfoPtr commandInfo)
{
	UNUSED_PARAM(commandInfo);
}

void xplCmdGetThreadIdEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	char buf[20];
	xmlNodePtr ret;

	snprintf(buf, sizeof(buf), XPR_THREAD_ID_FORMAT, xprGetCurrentThreadId());
	ret = xmlNewDocText(commandInfo->element->doc, BAD_CAST buf);
	ASSIGN_RESULT(ret, false, true);
}
