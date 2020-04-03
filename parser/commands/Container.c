#include "commands/Container.h"
#include "Core.h"
#include "Utils.h"

void xplCmdContainerPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdContainerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR BAD_CAST "repeat"
#define RETURNCONTENT_ATTR BAD_CAST "returncontent"

	BOOL repeat;
	BOOL return_content;
	xmlNodePtr error;

	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, FALSE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, RETURNCONTENT_ATTR, &return_content, TRUE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		return;
	}
	if (return_content)
	{
		ASSIGN_RESULT(detachContent(commandInfo->element), repeat, TRUE);
	} else {
		if (commandInfo->element->children)
			xplDocDeferNodeListDeletion(commandInfo->document, detachContent(commandInfo->element->children));
		ASSIGN_RESULT(NULL, FALSE, TRUE);
	}
}

xplCommand xplContainerCommand = { xplCmdContainerPrologue, xplCmdContainerEpilogue };
