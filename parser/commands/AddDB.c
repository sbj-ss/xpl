#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsession.h>
#include <libxpl/xpltree.h>
#include "commands/AddDB.h"

void xplCmdAddDBPrologue(xplCommandInfoPtr commandInfo)
{

}

void xplCmdAddDBEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")
#define CHECK_ATTR (BAD_CAST "check")

	xmlChar *name_attr = NULL;
	bool check;
	xmlChar *content = NULL;
	xmlNodePtr error;
	xplDBConfigResult cfg_result;

	if (!xplSessionGetSaMode(commandInfo->document->session))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), true, true);
		goto done;
	}

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	if (!name_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "no database name specified"), true, true);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, CHECK_ATTR, &check, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}

	if (!checkNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "connection string is non-text"), true, true);
		goto done;
	}
	content = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, 1);
	if (!content)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "connection string is empty"), true, true);
		goto done;
	}
	xplLockThreads(true);
	cfg_result = xplAddDB(name_attr, content, check);
	if (cfg_result == XPL_DBCR_OK)
		ASSIGN_RESULT(NULL, false, true);
	else
		ASSIGN_RESULT(xplCreateErrorNode(
			commandInfo->element, BAD_CAST "can't add database \"%s\": %s", name_attr, xplDecodeDBConfigResult(cfg_result)
		), true, true);
	xplLockThreads(false);
done:
	if (name_attr)
		XPL_FREE(name_attr);
	if (content)
		XPL_FREE(content);
}

xplCommand xplAddDBCommand = { xplCmdAddDBPrologue, xplCmdAddDBEpilogue };


