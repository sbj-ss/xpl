#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdBreakEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdBreakParams
{
	xmlXPathObjectPtr point;
} xplCmdBreakParams, *xplCmdBreakParamsPtr;

static const xplCmdBreakParams params_stencil =
{
	.point = NULL
};

xplCommand xplBreakCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdBreakEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdBreakParams),
	.parameters = {
		{
			.name = BAD_CAST "point",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_ANY,
			.value_stencil = &params_stencil.point
		}, {
			.name = NULL
		}
	}
};

static bool _checkForParent(xplDocumentPtr doc, xmlNodePtr element, xmlChar *xpath)
{
	xmlXPathObjectPtr sel;
	size_t i;
	bool ret = false;

	sel = xplSelectNodesWithCtxt(doc->xpath_ctxt, element->parent->parent, xpath);
	for (i = 0; i < (size_t) sel->nodesetval->nodeNr; i++)
		if (sel->nodesetval->nodeTab[i] == element->parent)
			ret = true;
	xmlXPathFreeObject(sel);
	return ret;
}

static xmlNodePtr _createBreak(xplCommandInfoPtr commandInfo, xmlChar *where)
{
	xmlNodePtr ret;
	xmlNsPtr xpl_ns;

	if (!(xpl_ns = commandInfo->document->root_xpl_ns))
		xpl_ns = xmlSearchNsByHref(commandInfo->element->doc, commandInfo->element, cfgXplNsUri);
	ret = xmlNewDocNode(commandInfo->element->doc, xpl_ns, BAD_CAST "break", NULL);
	xmlNewProp(ret, BAD_CAST "point", where);
	return ret;
}

void xplCmdBreakEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdBreakParamsPtr cmd_params = (xplCmdBreakParamsPtr) commandInfo->params;
	xmlChar number_buf[32];
	bool delete_tail = true;
	bool ascend = false;
	xmlChar *upper_point;
	xmlNodePtr upper_break;

	if (cmd_params->point)
	{
		switch (cmd_params->point->type)
		{
			case XPATH_BOOLEAN:
				delete_tail = cmd_params->point->boolval;
				break;
			case XPATH_STRING:
				delete_tail = (cmd_params->point->stringval && *cmd_params->point->stringval);
				break;
			case XPATH_NUMBER:
				ascend = (cmd_params->point->floatval > 1.0);
				if (ascend)
				{
					snprintf((char*) number_buf, sizeof(number_buf) - 1, "%d", (int) (cmd_params->point->floatval - .5));
					number_buf[sizeof(number_buf) - 1] = 0;
					upper_point = number_buf;
				}
				break;
			case XPATH_NODESET:
				if (cmd_params->point->nodesetval)
					ascend = !_checkForParent(commandInfo->document, commandInfo->element, cmd_params->point->user);
				else
					ascend = true;
				upper_point = (xmlChar*) cmd_params->point->user;
				break;
			default:
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "point XPath expression \"%s\" evaluated to invalid type", cmd_params->point->user), true, true);
				return;
		}
		if (ascend)
		{
			if (commandInfo->element->parent->parent &&
				commandInfo->element->parent->parent->type == XML_ELEMENT_NODE)
			{
				upper_break = _createBreak(commandInfo, upper_point);
				xmlAddNextSibling(commandInfo->element->parent, upper_break);
			} // TODO warning
		}
	}
	if (delete_tail)
	{
		xplDocDeferNodeListDeletion(commandInfo->document, commandInfo->element->next);
		commandInfo->element->next = NULL;
		commandInfo->element->parent->last = commandInfo->element;
	}
	ASSIGN_RESULT(NULL, false, true);
}
