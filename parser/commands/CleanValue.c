#include <libxpl/xplmessages.h>
#include <libxpl/xplparams.h>
#include <libxpl/xpltree.h>
#include "commands/CleanValue.h"

void xplCmdCleanValuePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdCleanValueEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define EXPECT_ATTR (BAD_CAST "expect")
#define REMOVEONNONMATCH_ATTR (BAD_CAST "removeonnonmatch")
	xmlChar *expect_attr = NULL;
	xplExpectType expect;
	bool remove_on_nonmatch;
	xmlNodePtr error, ret;
	xmlChar *value = NULL, *clean_value;

	expect_attr = xmlGetNoNsProp(commandInfo->element, EXPECT_ATTR);
	if (!expect_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "missing expect attribute"), true, true);
		goto done;
	}
	if ((expect = xplExpectTypeFromString(expect_attr)) == XPL_EXPECT_UNDEFINED)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "unknown expect value: \"%s\"", expect_attr), true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REMOVEONNONMATCH_ATTR, &remove_on_nonmatch, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if (!checkNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "non-text nodes inside"), true, true);
		goto done;
	}
	value = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, 1);
	if (!value)
	{
		ASSIGN_RESULT(NULL, false, true);
		goto done; 
	}
	clean_value = xplCleanTextValue(value, expect);
	if (remove_on_nonmatch && (xmlStrlen(value) != xmlStrlen(clean_value)))
	{
		XPL_FREE(clean_value);
		clean_value = NULL;
	}
	if (clean_value)
	{
		ret = xmlNewDocText(commandInfo->element->doc, NULL);
		ret->content = clean_value;
		ASSIGN_RESULT(ret, false, true);
	} else {
		ASSIGN_RESULT(NULL, false, true);
	}
done:
	if (expect_attr) XPL_FREE(expect_attr);
	if (value) XPL_FREE(value);
}

xplCommand xplCleanValueCommand = { xplCmdCleanValuePrologue, xplCmdCleanValueEpilogue };
