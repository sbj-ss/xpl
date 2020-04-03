#include "commands/CommandSupported.h"
#include "Utils.h"

void xplCmdCommandSupportedPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdCommandSupportedEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")
#define TAGNAME_ATTR (BAD_CAST "tagname")
#define REPEAT_ATTR (BAD_CAST "repeat")
#define SHOWTAGS_ATTR (BAD_CAST "showtags")

	xmlChar *name_attr = NULL;
	xmlChar *tagname_attr = NULL;
	xmlChar *name;
	BOOL repeat;
	BOOL showtags;
	xmlChar *value;
	xmlNodePtr ret, error;

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	tagname_attr = xmlGetNoNsProp(commandInfo->element, TAGNAME_ATTR);
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, TRUE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, SHOWTAGS_ATTR, &showtags, FALSE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		goto done;
	}
	if (!name_attr) /* all commands */
	{
		if (showtags)
			name = NULL;
		else if (tagname_attr)
			name = tagname_attr;
		else
			name = BAD_CAST "command";
		ASSIGN_RESULT(xplSupportedCommandsToList(commandInfo->element->doc, commandInfo->element, name), repeat, TRUE);
	} else { /* specified command */
		value = BAD_CAST (xplCommandSupported(name_attr)? "true": "false");
		ret = xmlNewDocText(commandInfo->element->doc, value);
		ASSIGN_RESULT(ret, FALSE, TRUE);
	}
done:
	if (name_attr) xmlFree(name_attr);
	if (tagname_attr) xmlFree(tagname_attr);
}

xplCommand xplCommandSupportedCommand = { xplCmdCommandSupportedPrologue, xplCmdCommandSupportedEpilogue };
