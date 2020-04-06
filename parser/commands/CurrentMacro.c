#include "commands/CurrentMacro.h"
#include "Macro.h"
#include "Messages.h"
#include "Core.h"
#include "Utils.h"

void xplCmdCurrentMacroPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdCurrentMacroEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define TAGNAME_ATTR (BAD_CAST "tagname")
#define REPEAT_ATTR (BAD_CAST "repeat")
#define DETAILED_ATTR (BAD_CAST "detailed")
	xmlChar *tagname_attr = NULL;
	bool repeat;
	bool detailed;
	xmlChar *tagname;
	xmlNodePtr error;

	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, true)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, DETAILED_ATTR, &detailed, false)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	tagname_attr = xmlGetNoNsProp(commandInfo->element, TAGNAME_ATTR);
	tagname = tagname_attr? tagname_attr: BAD_CAST "macro";
	if (!commandInfo->document->current_macro)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "no active macros"), true, true);
		goto done;
	}
	if (detailed)
	{
		ASSIGN_RESULT(xplMacroToNode(commandInfo->document->current_macro, tagname, commandInfo->element), repeat, true);
		downshiftNodeNsDef(result->list, commandInfo->element->nsDef);
	} else {
		ASSIGN_RESULT(xmlNewDocText(commandInfo->element->doc, commandInfo->document->current_macro->name), false, true);
	}
done:
	if (tagname_attr) xmlFree(tagname_attr);
}

xplCommand xplCurrentMacroCommand = { xplCmdCurrentMacroPrologue, xplCmdCurrentMacroEpilogue };

