#include <libxpl/xplcommand.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>

void xplCmdUriEncodeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplUriEncodeCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdUriEncodeEpilogue,
	.flags = XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE,
	.params_stencil = NULL
};

void xplCmdUriEncodeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlChar *uri;
	xmlNodePtr ret;

#ifdef _USE_LIBIDN
	if (commandInfo->content)
	{
		uri = xstrEncodeUriIdn(commandInfo->content);
		if (uri)
		{
			ret = xmlNewDocText(commandInfo->element->doc, NULL);
			ret->content = uri;
		} else
			ret = NULL;
		ASSIGN_RESULT(ret, false, true);
	} else
		ASSIGN_RESULT(NULL, false, true);
#else
	ASSIGN_RESULT(xplCreateErrorNode(commandInfo, BAD_CAST "Interpreter is compiled without IDN support"), true, true);
#endif
}
