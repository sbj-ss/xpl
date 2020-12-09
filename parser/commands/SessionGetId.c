#include <libxpl/xplcore.h>
#include <libxpl/xplsession.h>
#include "commands/SessionGetId.h"

void xplCmdSessionGetIdEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlNodePtr ret;
	xmlChar *id;

	id = xplSessionGetId(commandInfo->document->main->session);
	if (id)
		ret = xmlNewDocText(commandInfo->document->document, id);
	else
		ret = NULL;
	ASSIGN_RESULT(ret, false, true);
}

xplCommand xplSessionGetIdCommand = { NULL, xplCmdSessionGetIdEpilogue };
