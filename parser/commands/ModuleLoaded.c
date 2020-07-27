#include <libxpl/xplcommand.h>
#include "commands/ModuleLoaded.h"

void xplCmdModuleLoadedPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdModuleLoadedEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")
#define TAGNAME_ATTR (BAD_CAST "tagname")
#define SHOWTAGS_ATTR (BAD_CAST "showtags")
#define REPEAT_ATTR (BAD_CAST "repeat")
	xmlChar *name_attr = NULL;
	xmlChar *tagname_attr = NULL;
	xmlChar *name;
	bool showtags;
	bool repeat;
	xmlNodePtr ret, error;

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	tagname_attr = xmlGetNoNsProp(commandInfo->element, TAGNAME_ATTR);
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, SHOWTAGS_ATTR, &showtags, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, true)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if (name_attr)
	{
		ret = xmlNewDocText(commandInfo->element->doc, xplIsModuleLoaded(name_attr)? BAD_CAST "true": BAD_CAST "false");
		ASSIGN_RESULT(ret, false, true);
	} else {
		if (showtags)
			name = NULL;
		else if (tagname_attr)
			name = tagname_attr;
		else 
			name = "module";
		ret = xplLoadedModulesToNodeList(name, commandInfo->element);
		ASSIGN_RESULT(ret, repeat, true);
	}
done:
	if (name_attr)
		XPL_FREE(name_attr);
	if (tagname_attr)
		XPL_FREE(tagname_attr);
}

xplCommand xplModuleLoadedCommand = { xplCmdModuleLoadedPrologue, xplCmdModuleLoadedEpilogue };
