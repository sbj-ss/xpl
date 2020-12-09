#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include "commands/Rehash.h"

void xplCmdRehashEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (!xplSessionGetSaMode(commandInfo->document->session))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), true, true);
		return;
	}
	xplLockThreads(true);
	xplReadConfig();
	xplLockThreads(false);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplRehashCommand = { NULL, xplCmdRehashEpilogue };
