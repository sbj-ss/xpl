#include "commands/DBSession.h"
#include "Utils.h"

void xplCmdDBSessionPrologue(xplCommandInfoPtr commandInfo)
{
}

#define REPEAT_ATTR (BAD_CAST "repeat")
void xplCmdDBSessionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	BOOL repeat;
	xmlNodePtr error;

	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, FALSE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		return;
	}
	ASSIGN_RESULT(detachContent(commandInfo->element), repeat, TRUE);
}

xplCommand xplDBSessionCommand = { xplCmdDBSessionPrologue, xplCmdDBSessionEpilogue };
