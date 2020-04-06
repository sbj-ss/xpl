#include "commands/DBSession.h"
#include "Utils.h"

void xplCmdDBSessionPrologue(xplCommandInfoPtr commandInfo)
{
}

#define REPEAT_ATTR (BAD_CAST "repeat")
void xplCmdDBSessionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	bool repeat;
	xmlNodePtr error;

	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, false)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	ASSIGN_RESULT(detachContent(commandInfo->element), repeat, true);
}

xplCommand xplDBSessionCommand = { xplCmdDBSessionPrologue, xplCmdDBSessionEpilogue };
