#include <libxpl/xplcore.h>
#include <libxpl/xplsession.h>
#include "commands/GetSaMode.h"

xplCommand xplGetSaModeCommand =
{
	.prologue = xplCmdGetSaModePrologue,
	.epilogue = xplCmdGetSaModeEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdGetSaModePrologue(xplCommandInfoPtr commandInfo)
{
	UNUSED_PARAM(commandInfo);
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
	ASSIGN_RESULT(ret, false, true);
}
