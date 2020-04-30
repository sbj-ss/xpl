#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplutils.h>
#include "commands/RemoveDB.h"

void xplCmdRemoveDBPrologue(xplCommandInfoPtr commandInfo)
{

}

void xplCmdRemoveDBEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")

	xmlChar *name_attr = NULL;
	xplDBConfigResult cfg_result;

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);

	if (!xplSessionGetSaMode(commandInfo->document->session))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), true, true);
		goto done;
	}

	if (!name_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "no database name specified"), true, true);
		goto done;
	}
	xplLockThreads(TRUE);
	cfg_result = xplRemoveDB(name_attr);
	if (cfg_result == XPL_DBCR_OK)
		ASSIGN_RESULT(NULL, false, true);
	else
		ASSIGN_RESULT(xplCreateErrorNode(
			commandInfo->element, BAD_CAST "can't remove database \"%s\": %s", name_attr, xplDecodeDBConfigResult(cfg_result)
		), true, true);
	xplLockThreads(FALSE);
done:
	if (name_attr)
		xmlFree(name_attr);
}

xplCommand xplRemoveDBCommand = { xplCmdRemoveDBPrologue, xplCmdRemoveDBEpilogue };
