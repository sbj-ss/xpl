#include <libxpl/xplcommand.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>
#include <libxml/uri.h>

void xplCmdUriEscapeParamEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplUriEscapeParamCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdUriEscapeParamEpilogue,
	.flags = XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE,
	.params_stencil = NULL,
	.stencil_size = 0
};

void xplCmdUriEscapeParamEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xmlChar *param = NULL;
	xmlNodePtr ret;

	if (!commandInfo->content)
	{
		ASSIGN_RESULT(NULL, false, true);
		return;
	}
	ret = xmlNewDocText(commandInfo->element->doc, NULL);
	ret->content = xmlURIEscapeStr(commandInfo->content, NULL);
	ASSIGN_RESULT(ret, false, true);
}
