#include <libxpl/xplcommand.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>

void xplCmdParseXmlEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdParseXmlParams
{
	bool repeat;
} xplCmdParseXmlParams, *xplCmdParseXmlParamsPtr;

static const xplCmdParseXmlParams params_stencil =
{
	.repeat = true
};

xplCommand xplParseXmlCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdParseXmlEpilogue,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdParseXmlParams),
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE,
	.parameters = {
		{
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};

void xplCmdParseXmlEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdParseXmlParamsPtr params = (xplCmdParseXmlParamsPtr) commandInfo->params;
	xmlDocPtr doc = NULL;
	xmlChar *parse_error;
	xmlNodePtr ret, root;

	if (!commandInfo->content)
	{
		ASSIGN_RESULT(NULL, false, true);
		return;
	}
	doc = xmlParseMemory((const char*) commandInfo->content, xmlStrlen(commandInfo->content));
	if (!doc)
	{
		parse_error = xstrGetLastLibxmlError();
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "document parsing error: %s", parse_error), true, true);
		XPL_FREE(parse_error);
	} else {
		xplMergeDocOldNamespaces(doc, commandInfo->element->doc);
		root = ret = xplDetachChildren((xmlNodePtr) doc);
		while (root)
		{
			if (root->type == XML_ELEMENT_NODE)
				break;
		}
		if (root)
			xplLiftNsDefs(commandInfo->element->parent, root, root->children);
		xmlSetTreeDoc(ret, commandInfo->element->doc);
		xmlFreeDoc(doc);
		ASSIGN_RESULT(ret, params->repeat, true);
	}
}
