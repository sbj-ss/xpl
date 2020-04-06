#include "commands/Text.h"
#include "Core.h"
#include "Utils.h"

void xplCmdTextPrologue(xplCommandInfoPtr commandInfo)
{
	commandInfo->document->indent_spinlock++;
}

void xplCmdTextEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR (BAD_CAST "repeat")

	bool repeat;
	xmlNodePtr error;

	commandInfo->document->indent_spinlock--;
	if (!(commandInfo->element->type & XML_NODE_DELETION_MASK))
	{
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, false)))
		{
			ASSIGN_RESULT(error, true, true);
			return;
		} else
			ASSIGN_RESULT(detachContent(commandInfo->element), repeat, true);
	} else
		ASSIGN_RESULT(NULL, false, false);
}

xplCommand xplTextCommand = {
	SFINIT(.prologue, xplCmdTextPrologue), 
	SFINIT(.epilogue, xplCmdTextEpilogue),
	SFINIT(.initializer, NULL),
	SFINIT(.finalizer, NULL),
	SFINIT(.flags, XPL_CMD_FLAG_CONTENT_SAFE)
};
