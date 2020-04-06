#include "commands/Choose.h"
#include "Utils.h"

void xplCmdChoosePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdChooseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR (BAD_CAST "repeat")
	bool repeat;
	xmlNodePtr error;

	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, false)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	ASSIGN_RESULT(detachContent(commandInfo->element), repeat, true);
}

xplCommand xplChooseCommand = { xplCmdChoosePrologue, xplCmdChooseEpilogue };
