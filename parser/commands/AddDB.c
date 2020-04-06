#include "commands/AddDB.h"
#include "Core.h"
#include "Messages.h"
#include "Session.h"
#include "Utils.h"

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
	xplLockThreads(TRUE);
	switch (xplAddDB(name_attr, content, check))
	{
	case XPL_ADD_DB_OK:
		ASSIGN_RESULT(NULL, false, true);
		break;
	case XPL_ADD_DB_ALREADY_EXISTS:
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "database \"%s\" already exists", name_attr), true, true);
		break;
	case XPL_ADD_DB_INSUFFICIENT_MEMORY:
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "insufficient memory"), true, true);
		break;
	case XPL_ADD_DB_CHECK_FAILED:
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "couldn't connect to database \"%s\"", name_attr), true, true);
		break;
	default:
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "unknown error"), true, true);
		break;
	}
	xplLockThreads(FALSE);	
done:
	if (name_attr)
		xmlFree(name_attr);
	if (content)
		xmlFree(content);
}

xplCommand xplAddDBCommand = { xplCmdAddDBPrologue, xplCmdAddDBEpilogue };


