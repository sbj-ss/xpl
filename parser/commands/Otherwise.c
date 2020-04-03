#include "commands/Otherwise.h"
#include "Core.h"
#include "Options.h"
#include "Utils.h"

void xplCmdOtherwisePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdOtherwiseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR (BAD_CAST "repeat")
	BOOL repeat;
	xmlNodePtr brk, error;
	xmlNsPtr brk_ns;

	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, FALSE)))
	{
		ASSIGN_RESULT(error, TRUE, TRUE);
		return;
	}
	ASSIGN_RESULT(detachContent(commandInfo->element), repeat, TRUE);
	brk_ns = commandInfo->document->root_xpl_ns;
	if (!brk_ns)
		brk_ns = xmlSearchNsByHref(commandInfo->element->doc, commandInfo->element, cfgXplNsUri);
	brk = xmlNewDocNode(commandInfo->element->doc, brk_ns, BAD_CAST "break", NULL);
	appendList(commandInfo->element, brk);
}

xplCommand xplOtherwiseCommand = { xplCmdOtherwisePrologue, xplCmdOtherwiseEpilogue };
