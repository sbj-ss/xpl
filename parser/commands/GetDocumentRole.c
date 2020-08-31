#include <libxpl/xplcore.h>
#include <libxpl/xplwrappers.h>
#include "commands/GetDocumentRole.h"

xplCommand xplGetDocumentRoleCommand =
{
	.prologue = xplCmdGetDocumentRolePrologue,
	.epilogue = xplCmdGetDocumentRoleEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdGetDocumentRolePrologue(xplCommandInfoPtr commandInfo)
{
	UNUSED_PARAM(commandInfo);
}

void xplCmdGetDocumentRoleEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlNodePtr ret;

	ret = xmlNewDocText(commandInfo->element->doc, xplDocRoleToString(commandInfo->document->role));
	ASSIGN_RESULT(ret, false, true);
}
