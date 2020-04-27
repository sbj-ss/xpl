#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplutils.h>
#include "commands/UnloadModule.h"

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
	ASSIGN_RESULT(ret, true, true);
	if (name_attr) xmlFree(name_attr);
}

xplCommand xplUnloadModuleCommand = { xplCmdUnloadModulePrologue, xplCmdUnloadModuleEpilogue };
