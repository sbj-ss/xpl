#include <libxpl/xplcommand.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>

void xplCmdUriEncodeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

void xplCmdUriEncodeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlChar *content = NULL, *uri;
	xmlNodePtr ret;

	if (!xplCheckNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "element content is non-text"), true, true);
		return;
	}
#ifdef _USE_LIBIDN
	if ((content = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, 1)))
	{
		uri = xstrEncodeUriIdn(content);
		if (uri)
		{
			ret = xmlNewDocText(commandInfo->element->doc, NULL);
			ret->content = uri;
		} else
			ret = NULL;
		ASSIGN_RESULT(ret, false, true);
		XPL_FREE(content);
	} else
		ASSIGN_RESULT(NULL, false, true);
#else
	ASSIGN_RESULT(xplCreateErrorNode(commandInfo, BAD_CAST "Interpreter is compiled without IDN support"), true, true);
#endif
}

xplCommand xplUriEncodeCommand = { NULL, xplCmdUriEncodeEpilogue };