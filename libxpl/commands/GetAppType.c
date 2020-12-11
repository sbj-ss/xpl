#include <libxpl/xplcore.h>

void xplCmdGetAppTypeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplGetAppTypeCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdGetAppTypeEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdGetAppTypeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlNodePtr ret = xmlNewDocText(commandInfo->element->doc, xplGetAppType());
	ASSIGN_RESULT(ret, false, true);
}
