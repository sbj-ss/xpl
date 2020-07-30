#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsession.h>
#include <libxpl/xpltree.h>
#include "commands/GetDB.h"

void xplCmdGetDBPrologue(xplCommandInfoPtr commandInfo)
{

}

void xplCmdGetDBEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")
#define RESPONSETAGNAME_ATTR (BAD_CAST "responsetagname")
#define SHOWTAGS_ATTR (BAD_CAST "showtags")
#define REPEAT_ATTR (BAD_CAST "repeat")

	xmlChar *name_attr = NULL;
	xmlChar *responsetagname_attr = NULL;
	bool show_tags;
	bool repeat;
	xplDBListPtr db_list;
	xmlNodePtr ret, error;

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	responsetagname_attr = xmlGetNoNsProp(commandInfo->element, RESPONSETAGNAME_ATTR);
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, SHOWTAGS_ATTR, &show_tags, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, true)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}

	if (!xplSessionGetSaMode(commandInfo->document->session))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), true, true);
		goto done;
	}

	if (name_attr)
	{
		db_list = xplLocateDBList(name_attr);
		if (!db_list)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid database name \"%s\"", name_attr), true, true);
			goto done;
		}
		ret = xmlNewDocText(commandInfo->element->doc, db_list->conn_string);
		repeat = false;
	} else {
		ret = xplDatabasesToNodeList(commandInfo->element,
			responsetagname_attr?responsetagname_attr:BAD_CAST "Database", show_tags);
		downshiftNodeListNsDef(ret, commandInfo->element->nsDef);
	}
	ASSIGN_RESULT(ret, repeat, true);
done:
	if (name_attr) XPL_FREE(name_attr);
	if (responsetagname_attr) XPL_FREE(responsetagname_attr);
}

xplCommand xplGetDBCommand = { xplCmdGetDBPrologue, xplCmdGetDBEpilogue };


