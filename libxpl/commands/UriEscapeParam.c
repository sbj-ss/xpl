#include <libxpl/xplcommand.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>
#include <libxml/uri.h>

void xplCmdUriEscapeParamEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdUriEscapeParamParams
{
	xmlChar *encoding;
} xplCmdUriEscapeParamParams, *xplCmdUriEscapeParamParamsPtr;

static const xplCmdUriEscapeParamParams params_stencil =
{
	.encoding = BAD_CAST "utf-8"
};

xplCommand xplUriEscapeParamCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdUriEscapeParamEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdUriEscapeParamParams),
	.parameters = {
		{
			.name = BAD_CAST "encoding",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.encoding
		}, {
			.name = NULL
		}
	}
};

void xplCmdUriEscapeParamEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdUriEscapeParamParamsPtr params = (xplCmdUriEscapeParamParamsPtr) commandInfo->params;
	xmlChar *param = NULL;
	xmlNodePtr ret;

	if (!commandInfo->content)
	{
		ASSIGN_RESULT(NULL, false, true);
		return;
	}
	if (xmlStrcasecmp(params->encoding, BAD_CAST "utf-8"))
	{
		if (xstrIconvString(
			(char*) params->encoding,
			"utf-8",
			(char*) commandInfo->content,
			(char*) commandInfo->content + xmlStrlen(commandInfo->content),
			(char**) &param,
			NULL
		) == -1)
		{
			ASSIGN_RESULT(xplCreateErrorNode(
				commandInfo->element,
				BAD_CAST "cannot convert parameter value \"%s\" to encoding \"%s\"",
				commandInfo->content,
				params->encoding
			), true, true);
			return;
		}
	} else
		param = commandInfo->content;
	if (param)
	{
		ret = xmlNewDocText(commandInfo->element->doc, NULL);
		/* ToDo: there will be problems with utf-16 */
		ret->content = xmlURIEscapeStr(param, NULL);
		if (param != commandInfo->content)
			XPL_FREE(param);
	} else
		ret = NULL;
	ASSIGN_RESULT(ret, false, true);
}
