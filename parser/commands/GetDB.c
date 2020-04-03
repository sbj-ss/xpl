#include "commands/GetDB.h"
#include "Core.h"
#include "Messages.h"
#include "Session.h"
#include "Utils.h"

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
	BOOL show_tags;
	BOOL repeat;
	xplDBListPtr db_list;
	xmlNodePtr ret, error;

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	responsetagname_attr = xmlGetNoNsProp(commandInfo->element, RESPONSETAGNAME_ATTR);
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, SHOWTAGS_ATTR, &show_tags, FALSE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, TRUE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		goto done;
	}

	if (!xplSessionGetSaMode(commandInfo->document->session))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), TRUE, TRUE);
		goto done;
	}

	if (name_attr)
	{
		db_list = xplLocateDBList(name_attr);
		if (!db_list)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid database name \"%s\"", name_attr), TRUE, TRUE);
			goto done;
		}
		ret = xmlNewDocText(commandInfo->element->doc, db_list->conn_string);
		repeat = FALSE;
	} else {
		ret = xplDatabasesToNodeList(commandInfo->element->doc, commandInfo->element, 
			responsetagname_attr?responsetagname_attr:BAD_CAST "Database", show_tags);
		downshiftNodeListNsDef(ret, commandInfo->element->nsDef);
	}
	ASSIGN_RESULT(ret, repeat, TRUE);
done:
	if (name_attr) xmlFree(name_attr);
	if (responsetagname_attr) xmlFree(responsetagname_attr);
}

xplCommand xplGetDBCommand = { xplCmdGetDBPrologue, xplCmdGetDBEpilogue };


