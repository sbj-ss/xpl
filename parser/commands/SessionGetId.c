#include "commands/SessionGetId.h"
#include "Core.h"
#include "Session.h"
#include "Utils.h"

void xplCmdSessionGetIdPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdSessionGetIdEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlNodePtr ret;
	xmlChar *id;

	id = xplSessionGetId(commandInfo->document->main->session);
	if (id)
		ret = xmlNewDocText(commandInfo->document->document, id);
	else
		ret = NULL;
	ASSIGN_RESULT(ret, FALSE, TRUE);
}

xplCommand xplSessionGetIdCommand = { xplCmdSessionGetIdPrologue, xplCmdSessionGetIdEpilogue };
