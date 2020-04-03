#include "commands/SessionRemoveObject.h"
#include "Core.h"
#include "Messages.h"
#include "Session.h"
#include "Utils.h"

void xplCmdSessionRemoveObjectPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdSessionRemoveObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")
#define THREADLOCAL_ATTR (BAD_CAST "threadlocal")
	xmlChar *name_attr;
	BOOL threadlocal;
	xmlNodePtr error;

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	if (!name_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "no name attribute specified"), TRUE, TRUE);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, THREADLOCAL_ATTR, &threadlocal, FALSE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		goto done;
	}
	if (threadlocal)
		name_attr = appendThreadIdToString(name_attr, xprGetCurrentThreadId());
	xplSessionRemoveObject(commandInfo->document->main->session, name_attr);
	ASSIGN_RESULT(NULL, FALSE, TRUE);
done:
	if (name_attr) xmlFree(name_attr);
}

xplCommand xplSessionRemoveObjectCommand = { xplCmdSessionRemoveObjectPrologue, xplCmdSessionRemoveObjectEpilogue };
