#include "commands/GetDocumentRole.h"
#include "Core.h"
#include "Utils.h"
#include "Wrappers.h"

void xplCmdGetDocumentRolePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdGetDocumentRoleEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlNodePtr ret;

	ret = xmlNewDocText(commandInfo->element->doc, xplDocRoleToString(commandInfo->document->role));
	ASSIGN_RESULT(ret, FALSE, TRUE);
}

xplCommand xplGetDocumentRoleCommand = { xplCmdGetDocumentRolePrologue, xplCmdGetDocumentRoleEpilogue };
