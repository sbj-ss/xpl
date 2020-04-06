#include "commands/GetAppType.h"
#include "Core.h"
#include "Utils.h"


void xplCmdGetAppTypePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdGetAppTypeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlNodePtr ret = xmlNewDocText(commandInfo->element->doc, xplGetAppType());
	ASSIGN_RESULT(ret, false, true);
}

xplCommand xplGetAppTypeCommand = { xplCmdGetAppTypePrologue, xplCmdGetAppTypeEpilogue };
