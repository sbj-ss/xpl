#include "commands/UnloadModule.h"
#include "Core.h"
#include "Messages.h"
#include "Utils.h"

void xplCmdUnloadModulePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdUnloadModuleEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")

	xmlChar *name_attr;
	xmlNodePtr ret = NULL;

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	if (!name_attr)
	{
		ret = xplCreateErrorNode(commandInfo->element, BAD_CAST "no module name specified");
		goto done;
	}
	xplLockThreads(TRUE);
	xplUnloadModule(name_attr);
	xplLockThreads(FALSE);
done:
	ASSIGN_RESULT(ret, TRUE, TRUE);
	if (name_attr) xmlFree(name_attr);
}

xplCommand xplUnloadModuleCommand = { xplCmdUnloadModulePrologue, xplCmdUnloadModuleEpilogue };
