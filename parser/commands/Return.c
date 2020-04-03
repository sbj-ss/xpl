#include "commands/Return.h"
#include "Core.h"
#include "Messages.h"
#include "Utils.h"

void xplCmdReturnPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdReturnEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
/* TODO repeat */
	if (!commandInfo->document->current_macro)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "this command can only be called within macro definition"), TRUE, TRUE);
		return;
	}
	commandInfo->document->current_macro->return_value = detachContent(commandInfo->element);
	deleteNeighbours(commandInfo->element, commandInfo->document->current_macro->caller, TRUE);
	ASSIGN_RESULT(NULL, FALSE, TRUE);
}

xplCommand xplReturnCommand = { xplCmdReturnPrologue, xplCmdReturnEpilogue };
