#include <libxpl/xplcore.h>
#include <libxpl/xpltree.h>
#include "commands/Container.h"

void xplCmdContainerPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdContainerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR BAD_CAST "repeat"
#define RETURNCONTENT_ATTR BAD_CAST "returncontent"

	bool repeat;
	bool return_content;
	xmlNodePtr error;

	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, false)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, RETURNCONTENT_ATTR, &return_content, true)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	if (return_content)
	{
		ASSIGN_RESULT(xplDetachContent(commandInfo->element), repeat, true);
	} else {
		if (commandInfo->element->children)
			xplDocDeferNodeListDeletion(commandInfo->document, xplDetachContent(commandInfo->element->children));
		ASSIGN_RESULT(NULL, false, true);
	}
}

xplCommand xplContainerCommand = { xplCmdContainerPrologue, xplCmdContainerEpilogue };
