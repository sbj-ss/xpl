#include "commands/SessionSetObject.h"
#include "Core.h"
#include "Messages.h"
#include "Session.h"
#include "Utils.h"

void xplCmdSessionSetObjectPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdSessionSetObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR BAD_CAST "name"
#define THREADLOCAL_ATTR (BAD_CAST "threadlocal")
	xmlChar *name_attr = NULL;
	bool threadlocal;
	xmlNodePtr error;

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	if (!name_attr || !*name_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "name attribute not specified or empty"), true, true);
		goto done;
	}
	if (!commandInfo->document->main->session)
		commandInfo->document->main->session = xplSessionCreateWithAutoId();
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, THREADLOCAL_ATTR, &threadlocal, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if (threadlocal)
		name_attr = appendThreadIdToString(name_attr, xprGetCurrentThreadId());
	xplSessionSetObject(commandInfo->document->main->session, commandInfo->element, name_attr);
done:
	if (name_attr)
		xmlFree(name_attr);
	ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplSessionSetObjectCommand = { xplCmdSessionSetObjectPrologue, xplCmdSessionSetObjectEpilogue };
