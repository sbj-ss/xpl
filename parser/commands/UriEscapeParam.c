#include "commands/UriEscapeParam.h"
#include "Messages.h"
#include "Utils.h"
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

	if (!checkNodeListForText(commandInfo->element->children))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "element content is non-text"), TRUE, TRUE);
		return;
	}
	if ((content = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, 1)))
	{
		encoding_attr = xmlGetNoNsProp(commandInfo->element, ENCODING_ATTR);
		if (encoding_attr)
		{
			if (xmlStrcasecmp(encoding_attr, BAD_CAST "utf-8"))
			{
				if (iconv_string((char*) encoding_attr, "utf-8", content, content + xmlStrlen(content), (char**) &param, NULL) == -1)
				{
					ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "cannot convert parameter value \"%s\" to encoding \"%s\"", content, encoding_attr), TRUE, TRUE);
					xmlFree(content);
					xmlFree(encoding_attr);
					return;
				}
				xmlFree(content);
			} else
				param = content;
			xmlFree(encoding_attr);
		} else
			param = content;
		if (param)
		{
			ret = xmlNewDocText(commandInfo->element->doc, NULL);
			/* ToDo: there will be problems with utf-16 */
			ret->content = xmlURIEscapeStr(param, NULL);
			xmlFree(param);
		} else
			ret = NULL;
		ASSIGN_RESULT(ret, FALSE, TRUE);
	} else
		ASSIGN_RESULT(NULL, FALSE, TRUE);
}

xplCommand xplUriEscapeParamCommand = { xplCmdUriEscapeParamPrologue, xplCmdUriEscapeParamEpilogue };
