#include <libxpl/xplmacro.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include "commands/ListMacros.h"

void xplCmdListMacrosPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdListMacrosEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define TAGNAME_ATTR (BAD_CAST "tagname")
#define REPEAT_ATTR (BAD_CAST "repeat")
#define DELIMITER_ATTR (BAD_CAST "delimiter")
#define UNIQUE_ATTR (BAD_CAST "unique")
	xmlChar *tagname_attr = NULL;
	xmlChar *delimiter_attr = NULL;
	xmlChar *tagname;
	bool repeat;
	bool unique;
	xmlNodePtr ret, error;

	tagname_attr = xmlGetNoNsProp(commandInfo->element, TAGNAME_ATTR);
	tagname = tagname_attr? tagname_attr: BAD_CAST "macro";
	delimiter_attr = xmlGetNoNsProp(commandInfo->element, DELIMITER_ATTR);
	if (delimiter_attr && tagname_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "cannot use delimiter and tagname at the same time"), true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, true)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, UNIQUE_ATTR, &unique, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if (delimiter_attr)
	{
		ret = xmlNewDocText(commandInfo->element->doc, NULL);
		ret->content = xplMacroTableToString(commandInfo->element, delimiter_attr, unique);
	} else {
		ret = xplMacroTableToNodeList(commandInfo->element, tagname, unique, commandInfo->element);
		downshiftNodeListNsDef(ret, commandInfo->element->nsDef);
	}
	ASSIGN_RESULT(ret, repeat, true);
done:
	if (tagname_attr)
		xmlFree(tagname_attr);
	if (delimiter_attr)
		xmlFree(delimiter_attr);
}

xplCommand xplListMacrosCommand = { xplCmdListMacrosPrologue, xplCmdListMacrosEpilogue };
