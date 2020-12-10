#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdSwitchPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSwitchEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

void xplCmdSwitchPrologue(xplCommandInfoPtr commandInfo)
{
#define KEY_ATTR (BAD_CAST "key")
	xmlChar *key_attr;
	xmlXPathObjectPtr nodes = NULL;
	if ((key_attr = xmlGetNoNsProp(commandInfo->element, KEY_ATTR)))
	{
		nodes = xplSelectNodes(commandInfo, commandInfo->element, key_attr);
		if (!nodes)
			commandInfo->prologue_error = xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid key XPath expression \"%s\"", key_attr);
		XPL_FREE(key_attr);
	} else 
		commandInfo->prologue_error = xplCreateErrorNode(commandInfo->element, BAD_CAST "missing key attribute");
	if (commandInfo->prologue_error)
		xplDocDeferNodeListDeletion(commandInfo->document, xplDetachContent(commandInfo->element));
	else
		commandInfo->element->content = (xmlChar*) nodes;
}

void xplCmdSwitchEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR (BAD_CAST "repeat")
	bool repeat;
	xmlXPathObjectPtr nodes = (xmlXPathObjectPtr) commandInfo->element->content;
	xmlNodePtr error;

	if (commandInfo->element->type & XPL_NODE_DELETION_MASK)
	{
		if (nodes->nodesetval)
			nodes->nodesetval->nodeNr = 0;
		xmlXPathFreeObject(nodes);
		ASSIGN_RESULT(NULL, false, false);
	} else if (nodes) {
		xmlXPathFreeObject(nodes);
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, false)))
			ASSIGN_RESULT(error, true, true);
		else
			ASSIGN_RESULT(xplDetachContent(commandInfo->element), repeat, true);
	} else
		ASSIGN_RESULT(commandInfo->prologue_error, true, true);
}

xplCommand xplSwitchCommand = { 
	.prologue = xplCmdSwitchPrologue, 
	.epilogue = xplCmdSwitchEpilogue,
	.initializer = NULL,
	.finalizer = NULL,
	.flags = XPL_CMD_FLAG_CONTENT_SAFE
};
