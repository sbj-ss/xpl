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
	xplRemoveDB(name_attr);
	xplLockThreads(FALSE);
	ASSIGN_RESULT(NULL, false, true);
done:
	if (name_attr)
		xmlFree(name_attr);
}

xplCommand xplRemoveDBCommand = { xplCmdRemoveDBPrologue, xplCmdRemoveDBEpilogue };
