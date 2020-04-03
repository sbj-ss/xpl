#include "commands/Rehash.h"
#include "Core.h"
#include "Messages.h"
#include "Utils.h"

void xplCmdRehashPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdRehashEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (!xplSessionGetSaMode(commandInfo->document->session))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), TRUE, TRUE);
		return;
	}
	xplLockThreads(TRUE);
	xplReadConfig();
	xplLockThreads(FALSE);
	ASSIGN_RESULT(NULL, FALSE, TRUE);
}

xplCommand xplRehashCommand = { xplCmdRehashPrologue, xplCmdRehashEpilogue };
