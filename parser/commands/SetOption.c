#include "commands/SetOption.h"
#include "Core.h"
#include "Messages.h"
#include "Options.h"
#include "Session.h"
#include "Utils.h"

void xplCmdSetOptionPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdSetOptionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")
#define VALUE_ATTR (BAD_CAST "value")

	xmlChar *name_attr = NULL;
	xmlChar *value_attr = NULL;
	BOOL by_default = FALSE;

	if (!xplSessionGetSaMode(commandInfo->document->session))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), TRUE, TRUE);
		return;
	}

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	value_attr = xmlGetNoNsProp(commandInfo->element, VALUE_ATTR);

	if (!name_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "missing option name"), TRUE, TRUE);
		goto done;
	}
	if (!value_attr)
		by_default = TRUE;

	xplLockThreads(TRUE);
	switch (xplSetOptionValue(name_attr, value_attr, by_default))
	{
	case XPL_SET_OPTION_OK: 
		ASSIGN_RESULT(NULL, FALSE, TRUE);
		break;
	case XPL_SET_OPTION_UNKNOWN_OPTION: 
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "unknown option \"%s\"", name_attr), TRUE, TRUE);
		break;
	case XPL_SET_OPTION_INVALID_VALUE:
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid option value \"%s\"", value_attr), TRUE, TRUE);
		break;
	case XPL_SET_OPTION_INTERNAL_ERROR:
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "internal error, please contact the developer"), TRUE, TRUE);
		break;
	}
done:
	xplLockThreads(FALSE);
	if (name_attr) xmlFree(name_attr);
	if (value_attr) xmlFree(value_attr);
}

xplCommand xplSetOptionCommand = { xplCmdSetOptionPrologue, xplCmdSetOptionEpilogue };
