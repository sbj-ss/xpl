#include <libxpl/xplcore.h>

void xplCmdGetSaModeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplGetSaModeCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdGetSaModeEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdGetSaModeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlNodePtr ret;
	xmlChar *txt;

	if (xplDocSessionGetSaMode(commandInfo->document))
		txt = BAD_CAST "true";
	else 
		txt = BAD_CAST "false";
	ret = xmlNewDocText(commandInfo->element->doc, txt);
	ASSIGN_RESULT(ret, false, true);
}
