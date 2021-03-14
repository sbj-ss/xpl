#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdTestEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdTestParams
{
	bool repeat;
	xmlChar *point;
} xplCmdTestParams, *xplCmdTestParamsPtr;

static const xplCmdTestParams params_stencil =
{
	.repeat = true,
	.point = NULL
};

xplCommand xplTestCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdTestEpilogue,
	.flags = XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE | XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_REQUIRE_CONTENT,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdTestParams),
	.parameters = {
		{
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = BAD_CAST "point",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.point
		}, {
			.name = NULL
		}
	}
};

static xmlNodePtr _createBreak(xplCommandInfoPtr commandInfo, xmlNodePtr error, xmlChar *point)
{
	xmlNsPtr xpl_ns;
	xmlNodePtr ret;

	if (!(xpl_ns = commandInfo->document->root_xpl_ns))
		xpl_ns = xmlSearchNsByHref(commandInfo->element->doc, commandInfo->element, cfgXplNsUri);
	ret = xmlNewDocNode(commandInfo->element->doc, xpl_ns, BAD_CAST "break", NULL);
	if (point)
		xmlNewProp(ret, BAD_CAST "point", point);
	if (error)
	{
		xmlAddNextSibling(error, ret);
		ret = error;
	}
	return ret;
}

void xplCmdTestEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdTestParamsPtr params = (xplCmdTestParamsPtr) commandInfo->params;
	xmlXPathObjectPtr ct = NULL;
	int smth = 0;
	xmlNodePtr error, parent;

	ct = xplSelectNodes(commandInfo, commandInfo->element, commandInfo->content);
	if (ct)
	{
		switch(ct->type)
		{
			case XPATH_NODESET:
				smth = (ct->nodesetval)? ct->nodesetval->nodeNr: 0;
				break;
			case XPATH_BOOLEAN:
				smth = ct->boolval;
				break;
			case XPATH_NUMBER:
				smth = (ct->floatval != 0.0);
				break;
			case XPATH_STRING:
				smth = (ct->stringval && *ct->stringval);
				break;
			default:
				error = xplCreateErrorNode(commandInfo->element, "unsupported XPath result type (expression is '%s')", commandInfo->content);
				ASSIGN_RESULT(_createBreak(commandInfo, error, NULL), true, true);
				goto done;
		}
		if (!smth)
			ASSIGN_RESULT(_createBreak(commandInfo, NULL, params->point), params->repeat, true);
		else {
			ASSIGN_RESULT(NULL, false, true);
			parent = commandInfo->element->parent;
			if (parent && parent->ns && !xmlStrcmp(parent->name, BAD_CAST("when")) && xplCheckNodeForXplNs(commandInfo->document, parent))
				xmlAddNextSibling(parent, _createBreak(commandInfo, NULL, NULL));
		}
	} else {
		error = xplCreateErrorNode(commandInfo->element, "invalid XPath expression '%s'", commandInfo->content);
		ASSIGN_RESULT(_createBreak(commandInfo, error, NULL), true, true);
	}
done:
	if (ct)
		xmlXPathFreeObject(ct);
}
