#include <libxpl/xplcore.h>
#include <libxpl/xplwrappers.h>
#include "commands/GetDocumentRole.h"

void xplCmdGetDocumentRolePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdGetDocumentRoleEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlNodePtr ret;

	ret = xmlNewDocText(commandInfo->element->doc, xplDocRoleToString(commandInfo->document->role));
	ASSIGN_RESULT(ret, false, true);
}

xplCommand xplGetDocumentRoleCommand = { xplCmdGetDocumentRolePrologue, xplCmdGetDocumentRoleEpilogue };
