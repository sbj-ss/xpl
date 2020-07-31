#include <libxpl/xplmessages.h>
#include "commands/LoadModule.h"

XPR_MUTEX load_module_locker;

void xplCmdLoadModulePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdLoadModuleEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")

	xmlChar *name_attr = NULL;
	xmlChar *error;
	xmlNodePtr ret = NULL;
	xplModuleCmdResult code;

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	if (!name_attr)
	{
		ret = xplCreateErrorNode(commandInfo->element, BAD_CAST "no module name specified");
		goto done;
	}
	/* we aren't using xplLockThreads here : locking inside xplLoadModule() should be enough */
	code = xplLoadModule(name_attr, &error);
	if ((code == XPL_MODULE_CMD_OK) || (code == XPL_MODULE_CMD_MODULE_ALREADY_LOADED))
		goto done;
	ret = xplCreateErrorNode(commandInfo->element, BAD_CAST "cannot load module %s: %s", name_attr, error);
done:
	ASSIGN_RESULT(ret, true, true);
	if (error)
		XPL_FREE(error);
	if (name_attr)
		XPL_FREE(name_attr);
}

xplCommand xplLoadModuleCommand = 
{ 
	.prologue = xplCmdLoadModulePrologue,
	.epilogue = xplCmdLoadModuleEpilogue,
	.initializer = NULL, 
	.finalizer = NULL
};
