#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>
#include "commands/UriEscapeParam.h"
#include "libxml/uri.h"

void xplCmdUriEscapeParamPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdUriEscapeParamEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define ENCODING_ATTR (BAD_CAST "encoding")
	xmlChar *content, *param = NULL;
	xmlChar *encoding_attr;
	xmlNodePtr ret;

	if (!xplCheckNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "element content is non-text"), true, true);
		return;
	}
	if ((content = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, 1)))
	{
		encoding_attr = xmlGetNoNsProp(commandInfo->element, ENCODING_ATTR);
		if (encoding_attr)
		{
			if (xmlStrcasecmp(encoding_attr, BAD_CAST "utf-8"))
			{
				if (xstrIconvString((char*) encoding_attr, "utf-8", (char*) content, (char*) content + xmlStrlen(content), (char**) &param, NULL) == -1)
				{
					ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "cannot convert parameter value \"%s\" to encoding \"%s\"", content, encoding_attr), true, true);
					XPL_FREE(content);
					XPL_FREE(encoding_attr);
					return;
				}
				XPL_FREE(content);
			} else
				param = content;
			XPL_FREE(encoding_attr);
		} else
			param = content;
		if (param)
		{
			ret = xmlNewDocText(commandInfo->element->doc, NULL);
			/* ToDo: there will be problems with utf-16 */
			ret->content = xmlURIEscapeStr(param, NULL);
			XPL_FREE(param);
		} else
			ret = NULL;
		ASSIGN_RESULT(ret, false, true);
	} else
		ASSIGN_RESULT(NULL, false, true);
}

xplCommand xplUriEscapeParamCommand = { xplCmdUriEscapeParamPrologue, xplCmdUriEscapeParamEpilogue };
