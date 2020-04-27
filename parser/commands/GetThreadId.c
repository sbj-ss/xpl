#include <stdio.h>
#include <libxpl/xplutils.h>
#include "commands/GetThreadId.h"

void xplCmdGetThreadIdPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdGetThreadIdEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	XPR_THREAD_ID id;
	char buf[9];
	xmlNodePtr ret;

	id = xprGetCurrentThreadId();
	snprintf(buf, 9, XPR_THREAD_ID_FORMAT, id);
	ret = xmlNewDocText(commandInfo->element->doc, BAD_CAST buf);
	ASSIGN_RESULT(ret, false, true);
}

xplCommand xplGetThreadIdCommand = { xplCmdGetThreadIdPrologue, xplCmdGetThreadIdEpilogue };
