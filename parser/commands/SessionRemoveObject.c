#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsession.h>
#include <libxpl/xplstring.h>
#include "commands/SessionRemoveObject.h"

void xplCmdSessionRemoveObjectPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdSessionRemoveObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")
#define THREADLOCAL_ATTR (BAD_CAST "threadlocal")
	xmlChar *name_attr;
	bool threadlocal;
	xmlNodePtr error;

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	if (!name_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "no name attribute specified"), true, true);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, THREADLOCAL_ATTR, &threadlocal, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if (threadlocal)
		name_attr = appendThreadIdToString(name_attr, xprGetCurrentThreadId());
	xplSessionRemoveObject(commandInfo->document->main->session, name_attr);
	ASSIGN_RESULT(NULL, false, true);
done:
	if (name_attr) XPL_FREE(name_attr);
}

xplCommand xplSessionRemoveObjectCommand = { xplCmdSessionRemoveObjectPrologue, xplCmdSessionRemoveObjectEpilogue };
