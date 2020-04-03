#include "commands/UriEncode.h"
#include "Messages.h"
#include "Utils.h"

void xplCmdUriEncodePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdUriEncodeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlChar *content = NULL, *uri;
	xmlNodePtr ret;

	if (!checkNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "element content is non-text"), TRUE, TRUE);
		return;
	}
#ifdef _USE_LIBIDN
	if ((content = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, 1)))
	{
		uri = encodeUriIdn(content);
		if (uri)
		{
			ret = xmlNewDocText(commandInfo->element->doc, NULL);
			ret->content = uri;
		} else
			ret = NULL;
		ASSIGN_RESULT(ret, FALSE, TRUE);
		xmlFree(content);
	} else
		ASSIGN_RESULT(NULL, FALSE, TRUE);
#else
	ASSIGN_RESULT(xplCreateErrorNode(commandInfo, BAD_CAST "Interpreter is compiled without IDN support"), TRUE, TRUE);
#endif
}

xplCommand xplUriEncodeCommand = { xplCmdUriEncodePrologue, xplCmdUriEncodeEpilogue };
