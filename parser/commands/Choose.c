#include "commands/Choose.h"
#include "Utils.h"

void xplCmdChoosePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdChooseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR (BAD_CAST "repeat")
	BOOL repeat;
	xmlNodePtr error;

	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, FALSE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		return;
	}
	ASSIGN_RESULT(detachContent(commandInfo->element), repeat, TRUE);
}

xplCommand xplChooseCommand = { xplCmdChoosePrologue, xplCmdChooseEpilogue };
