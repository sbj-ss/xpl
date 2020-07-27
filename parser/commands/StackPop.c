#include <libxpl/xplcore.h>
#include "commands/StackPop.h"

void xplCmdStackPopPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdStackPopEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR (BAD_CAST "repeat")
	xmlChar *repeat_attr = NULL;
	bool repeat = true;

	repeat_attr = xmlGetNoNsProp(commandInfo->element, REPEAT_ATTR);
	if (repeat_attr && !xmlStrcasecmp(repeat_attr, BAD_CAST "false"))
		repeat = false;
	ASSIGN_RESULT(xplPopFromDocStack(commandInfo->document, commandInfo->element->parent), repeat, true);
	if (repeat_attr)
		XPL_FREE(repeat_attr);
}

xplCommand xplStackPopCommand = { xplCmdStackPopPrologue, xplCmdStackPopEpilogue };
