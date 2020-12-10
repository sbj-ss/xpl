#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>

void xplCmdUnloadModuleEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

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
	xplLockThreads(true);
	xplUnloadModule(name_attr);
	xplLockThreads(false);
done:
	ASSIGN_RESULT(ret, true, true);
	if (name_attr) XPL_FREE(name_attr);
}

xplCommand xplUnloadModuleCommand = { NULL, xplCmdUnloadModuleEpilogue };
