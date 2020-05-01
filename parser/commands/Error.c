#include <libxpl/xplmessages.h>
#include "commands/Error.h"

void xplCmdErrorPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdErrorEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlChar *txt = NULL;
	/* this command is like :fatal, we can't make checks here and return errors other than specified */
	txt = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, true);
	ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "%s", txt), true, true);
	if (txt) xmlFree(txt);
}

xplCommand xplErrorCommand = { xplCmdErrorPrologue, xplCmdErrorEpilogue };
