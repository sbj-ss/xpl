#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplsession.h>
#include "commands/SetOption.h"

void xplCmdSetOptionPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdSetOptionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")
#define VALUE_ATTR (BAD_CAST "value")

	xmlChar *name_attr = NULL;
	xmlChar *value_attr = NULL;
	bool by_default = false;

	if (!xplSessionGetSaMode(commandInfo->document->session))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "access denied"), true, true);
		return;
	}

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	value_attr = xmlGetNoNsProp(commandInfo->element, VALUE_ATTR);

	if (!name_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "missing option name"), true, true);
		goto done;
	}
	if (!value_attr)
		by_default = true;

	xplLockThreads(true);
	switch (xplSetOptionValue(name_attr, value_attr, by_default))
	{
	case XPL_SET_OPTION_OK: 
		ASSIGN_RESULT(NULL, false, true);
		break;
	case XPL_SET_OPTION_UNKNOWN_OPTION: 
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "unknown option \"%s\"", name_attr), true, true);
		break;
	case XPL_SET_OPTION_INVALID_VALUE:
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid option value \"%s\"", value_attr), true, true);
		break;
	case XPL_SET_OPTION_INTERNAL_ERROR:
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "internal error, please contact the developer"), true, true);
		break;
	}
done:
	xplLockThreads(false);
	if (name_attr) XPL_FREE(name_attr);
	if (value_attr) XPL_FREE(value_attr);
}

xplCommand xplSetOptionCommand = { xplCmdSetOptionPrologue, xplCmdSetOptionEpilogue };
