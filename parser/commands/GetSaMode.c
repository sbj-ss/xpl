#include "commands/GetSaMode.h"
#include "Core.h"
#include "Session.h"
#include "Utils.h"

void xplCmdGetSaModePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdGetSaModeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlNodePtr ret;
	xmlChar *txt;

	if (xplSessionGetSaMode(commandInfo->document->session))
		txt = BAD_CAST "true";
	else 
		txt = BAD_CAST "false";
	ret = xmlNewDocText(commandInfo->element->doc, txt);
	ASSIGN_RESULT(ret, FALSE, TRUE);
}

xplCommand xplGetSaModeCommand = { xplCmdGetSaModePrologue, xplCmdGetSaModeEpilogue };
