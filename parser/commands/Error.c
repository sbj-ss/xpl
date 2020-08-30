#include <libxpl/xplmessages.h>
#include "commands/Error.h"

xplCommand xplErrorCommand =
{
	.prologue = xplCmdErrorPrologue,
	.epilogue = xplCmdErrorEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdErrorPrologue(xplCommandInfoPtr commandInfo)
{
	UNUSED_PARAM(commandInfo);
}

void xplCmdErrorEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlChar *txt = NULL;
	/* this command is like :fatal, we can't make checks here and return errors other than specified */
	txt = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, true);
	ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "%s", txt), true, true);
	if (txt) XPL_FREE(txt);
}
