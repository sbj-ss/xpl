#include "commands/SetParam.h"
#include "Core.h"
#include "Messages.h"
#include "Params.h"
#include "Utils.h"

#define SP_MODE_ADD 1
#define SP_MODE_REPLACE 2

void xplCmdSetParamPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdSetParamEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")
#define MODE_ATTR (BAD_CAST "mode")
	xmlChar *name_attr = NULL;
	xmlChar *mode_attr = NULL;
	int mode = SP_MODE_REPLACE;
	xmlChar *txt;
	xplParamResult res;
	xmlNodePtr err_node = NULL;

	ASSIGN_RESULT(NULL, false, true);
	if (!commandInfo->document->environment)
		goto done;
	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	if (!name_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "missing name attribute"), true, true);
		goto done;
	}
	mode_attr = xmlGetNoNsProp(commandInfo->element, MODE_ATTR);
	if (mode_attr)
	{
		if (!xmlStrcasecmp(mode_attr, BAD_CAST "add"))
			mode = SP_MODE_ADD;
		else if (!xmlStrcasecmp(mode_attr, BAD_CAST "replace"))
			mode = SP_MODE_REPLACE;
		else {
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "unknown mode: \"%s\"", mode_attr), true, true);
			goto done;
		}
	}
	if (!checkNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "non-text nodes inside"), true, true);
		goto done;
	}
	txt = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, true);
	switch (mode)
	{
		case SP_MODE_ADD: res = xplParamAddValue(commandInfo->document->environment, name_attr, txt, XPL_PARAM_TYPE_USERDATA); break;
		case SP_MODE_REPLACE: res = xplParamReplaceValue(commandInfo->document->environment, name_attr, txt, XPL_PARAM_TYPE_USERDATA); break;		
	}
	switch (res)
	{
	case XPL_PARAM_RES_OK:
		break;
	case XPL_PARAM_RES_OUT_OF_MEMORY:
		err_node = xplCreateErrorNode(commandInfo->element, BAD_CAST "out of memory");
		break;
	case XPL_PARAM_RES_INVALID_INPUT:
		err_node = xplCreateErrorNode(commandInfo->element, BAD_CAST "internal interpreter error, possibly out of memory");
		break;
	case XPL_PARAM_RES_TYPE_CLASH:
		err_node = xplCreateErrorNode(commandInfo->element, BAD_CAST "parameter with the same name (%s), but of another type already exists", name_attr);
		break;
	case XPL_PARAM_RES_READ_ONLY:
		err_node = xplCreateErrorNode(commandInfo->element, BAD_CAST "parameter \"%s\" is read-only", name_attr);
		break;
	case XPL_PARAM_RES_INTERNAL_ERROR:
		err_node = xplCreateErrorNode(commandInfo->element, BAD_CAST "internal hash error, possibly out of memory");
		break;
	default:
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		err_node = xplCreateErrorNode(commandInfo->element, BAD_CAST "internal error");
	}
	if (err_node)
	{
		xmlFree(txt);
		ASSIGN_RESULT(err_node, true, true);
	}
done:
	if (name_attr) xmlFree(name_attr);
	if (mode_attr) xmlFree(mode_attr);
}

xplCommand xplSetParamCommand = {xplCmdSetParamPrologue, xplCmdSetParamEpilogue };
