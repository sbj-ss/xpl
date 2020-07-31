#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include "commands/Return.h"

void xplCmdReturnPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdReturnEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
/* TODO repeat */
	if (!commandInfo->document->current_macro)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "this command can only be called within macro definition"), true, true);
		return;
	}
	commandInfo->document->current_macro->return_value = xplDetachContent(commandInfo->element);
	xplDeleteNeighbours(commandInfo->element, commandInfo->document->current_macro->caller, true);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplReturnCommand = { xplCmdReturnPrologue, xplCmdReturnEpilogue };
