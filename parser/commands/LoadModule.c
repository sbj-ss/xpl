#include "commands/LoadModule.h"
#include "Messages.h"
#include "Utils.h"

XPR_MUTEX load_module_locker;

void xplCmdLoadModulePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdLoadModuleEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")

	xmlChar *name_attr = NULL;
	xmlChar *error_data = NULL;
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
	code = xplLoadModule(name_attr, &error_data);
	if ((code == XPL_MODULE_CMD_OK) || (code == XPL_MODULE_CMD_MODULE_ALREADY_LOADED))
		goto done;
	error = xplModuleCmdResultToString(code, error_data);
	ret = xplCreateErrorNode(commandInfo->element, BAD_CAST "cannot load module %s: %s", name_attr, error);
	if (error)
		xmlFree(error);
	if (error_data)
		xmlFree(error_data);
done:
	ASSIGN_RESULT(ret, TRUE, TRUE);
	if (name_attr)
		xmlFree(name_attr);
}

xplCommand xplLoadModuleCommand = 
{ 
	SFINIT(.prologue, xplCmdLoadModulePrologue),
	SFINIT(.epilogue, xplCmdLoadModuleEpilogue),
	SFINIT(.initializer, NULL), 
	SFINIT(.finalizer, NULL)
};
