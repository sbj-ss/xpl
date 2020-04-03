#include "commands/GetOption.h"
#include "Core.h"
#include "Messages.h"
#include "Options.h"
#include "Session.h"
#include "Utils.h"

void xplCmdGetOptionPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdGetOptionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")
#define RESPONSETAGNAME_ATTR (BAD_CAST "responsetagname")
#define SHOWTAGS_ATTR (BAD_CAST "showtags")
#define SHOWPASSWORDS_ATTR (BAD_CAST "showpasswords")
#define REPEAT_ATTR (BAD_CAST "repeat")

	xmlChar *name_attr = NULL;
	xmlChar *responsetagname_attr = NULL;
	BOOL show_tags;
	BOOL show_passwords;
	BOOL repeat;
	xmlNodePtr ret, error;

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	responsetagname_attr = xmlGetNoNsProp(commandInfo->element, RESPONSETAGNAME_ATTR);
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, SHOWTAGS_ATTR, &show_tags, FALSE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, SHOWPASSWORDS_ATTR, &show_passwords, FALSE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, TRUE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		goto done;
	}

	if (show_passwords && !xplSessionGetSaMode(commandInfo->document->session))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), TRUE, TRUE);
		goto done;
	}

	if (name_attr)
	{
		ret = xmlNewDocText(commandInfo->element->doc, NULL);
		ret->content = xplGetOptionValue(name_attr, show_passwords);
		repeat = FALSE;
	} else {
		ret = xplOptionsToList(commandInfo->element->doc, commandInfo->element, 
			responsetagname_attr?responsetagname_attr:BAD_CAST "Option", show_tags, show_passwords);
		downshiftNodeListNsDef(ret, commandInfo->element->nsDef);
	}
	ASSIGN_RESULT(ret, repeat, TRUE);
done:
	if (name_attr)
		xmlFree(name_attr);
	if (responsetagname_attr)
		xmlFree(responsetagname_attr);
}

xplCommand xplGetOptionCommand = { xplCmdGetOptionPrologue, xplCmdGetOptionEpilogue };
