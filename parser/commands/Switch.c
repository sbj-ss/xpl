#include "commands/Switch.h"
#include "Core.h"
#include "Messages.h"
#include "Utils.h"

void xplCmdSwitchPrologue(xplCommandInfoPtr commandInfo)
{
#define KEY_ATTR (BAD_CAST "key")
	xmlChar *key_attr;
	xmlXPathObjectPtr nodes = NULL;
	if ((key_attr = xmlGetNoNsProp(commandInfo->element, KEY_ATTR)))
	{
		nodes = xplSelectNodes(commandInfo->document, commandInfo->element, key_attr);
		if (!nodes)
			commandInfo->_private = xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid key XPath expression \"%s\"", key_attr);
		xmlFree(key_attr);
	} else 
		commandInfo->_private = xplCreateErrorNode(commandInfo->element, BAD_CAST "missing key attribute");
	if (commandInfo->_private)
		xplDocDeferNodeListDeletion(commandInfo->document, detachContent(commandInfo->element));
	else
		commandInfo->element->content = (xmlChar*) nodes;
}

void xplCmdSwitchEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR (BAD_CAST "repeat")
	BOOL repeat;
	xmlXPathObjectPtr nodes = (xmlXPathObjectPtr) commandInfo->element->content;
	xmlNodePtr error;

	if (commandInfo->element->type & XML_NODE_DELETION_MASK)
	{
		if (nodes->nodesetval)
			nodes->nodesetval->nodeNr = 0;
		xmlXPathFreeObject(nodes);
		ASSIGN_RESULT(NULL, FALSE, FALSE);
	} else if (nodes) {
		xmlXPathFreeObject(nodes);
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, FALSE)))
			ASSIGN_RESULT(error, TRUE, TRUE);
		else
			ASSIGN_RESULT(detachContent(commandInfo->element), repeat, TRUE);
	} else
		ASSIGN_RESULT((xmlNodePtr) commandInfo->_private, TRUE, TRUE);
}

xplCommand xplSwitchCommand = { 
	SFINIT(.prologue, xplCmdSwitchPrologue), 
	SFINIT(.epilogue, xplCmdSwitchEpilogue),
	SFINIT(.initializer, NULL),
	SFINIT(.finalizer, NULL),
	SFINIT(.flags, XPL_CMD_FLAG_CONTENT_SAFE)
};
