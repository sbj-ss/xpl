#include <libxpl/xplcore.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdOtherwiseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

void xplCmdOtherwiseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR (BAD_CAST "repeat")
	bool repeat;
	xmlNodePtr brk, error;
	xmlNsPtr brk_ns;

	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, false)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	ASSIGN_RESULT(xplDetachContent(commandInfo->element), repeat, true);
	brk_ns = commandInfo->document->root_xpl_ns;
	if (!brk_ns)
		brk_ns = xmlSearchNsByHref(commandInfo->element->doc, commandInfo->element, cfgXplNsUri);
	brk = xmlNewDocNode(commandInfo->element->doc, brk_ns, BAD_CAST "break", NULL);
	xplAppendList(commandInfo->element, brk);
}

xplCommand xplOtherwiseCommand = { NULL, xplCmdOtherwiseEpilogue };
