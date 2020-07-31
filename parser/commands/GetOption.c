#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplsession.h>
#include <libxpl/xpltree.h>
#include "commands/GetOption.h"

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
	bool show_tags;
	bool show_passwords;
	bool repeat;
	xmlNodePtr ret, error;

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	responsetagname_attr = xmlGetNoNsProp(commandInfo->element, RESPONSETAGNAME_ATTR);
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, SHOWTAGS_ATTR, &show_tags, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, SHOWPASSWORDS_ATTR, &show_passwords, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, true)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}

	if (show_passwords && !xplSessionGetSaMode(commandInfo->document->session))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), true, true);
		goto done;
	}

	if (name_attr)
	{
		ret = xmlNewDocText(commandInfo->element->doc, NULL);
		ret->content = xplGetOptionValue(name_attr, show_passwords);
		repeat = false;
	} else {
		ret = xplOptionsToList(commandInfo->element->doc, commandInfo->element, 
			responsetagname_attr?responsetagname_attr:BAD_CAST "Option", show_tags, show_passwords);
		xplDownshiftNodeListNsDef(ret, commandInfo->element->nsDef);
	}
	ASSIGN_RESULT(ret, repeat, true);
done:
	if (name_attr)
		XPL_FREE(name_attr);
	if (responsetagname_attr)
		XPL_FREE(responsetagname_attr);
}

xplCommand xplGetOptionCommand = { xplCmdGetOptionPrologue, xplCmdGetOptionEpilogue };
