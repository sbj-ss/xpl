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

static bool checkForAncestor(xmlNodeSetPtr set, xmlNodePtr ancestor)
{
	size_t i;

	for (i = 0; i < (size_t) set->nodeNr; i++)
		if (set->nodeTab[i] == ancestor)
			return true;
	return false;
}

static xmlNodePtr createBreak(xplCommandInfoPtr commandInfo, xmlChar *where)
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
	bool do_ascend = true;
	bool do_climb = false;
	xmlChar *upper_point;
	xmlNodePtr upper_break;

	if (cmd_params->point)
	{
		switch (cmd_params->point->type)
		{
			case XPATH_BOOLEAN:
				do_ascend = cmd_params->point->boolval;
				break;
			case XPATH_STRING:
				do_ascend = (cmd_params->point->stringval && *cmd_params->point->stringval);
				break;
			case XPATH_NUMBER:
				do_climb = (cmd_params->point->floatval > 1.0);
				if (do_climb)
				{
					snprintf((char*) number_buf, sizeof(number_buf) - 1, "%d", (int) cmd_params->point->floatval - 1);
					number_buf[sizeof(number_buf) - 1] = 0;
					upper_point = number_buf;
				}
				break;
			case XPATH_NODESET:
				if (cmd_params->point->nodesetval && cmd_params->point->nodesetval->nodeNr)
					do_climb = !checkForAncestor(cmd_params->point->nodesetval, commandInfo->element->parent);
				else
					do_climb = true;
				upper_point = (xmlChar*) cmd_params->point->user;
				break;
			default:
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "point XPath expression \"%s\" evaluated to invalid type", cmd_params->point->user), true, true);
				return;
		}
		if (do_climb)
		{
			if (commandInfo->element->parent->parent &&
				commandInfo->element->parent->parent->type == XML_ELEMENT_NODE)
			{
				upper_break = createBreak(commandInfo, upper_point);
				xmlAddNextSibling(commandInfo->element->parent, upper_break);
			}
		}
	}
	if (do_ascend)
	{
		xplDocDeferNodeListDeletion(commandInfo->document, commandInfo->element->next);
		commandInfo->element->next = NULL;
		commandInfo->element->parent->last = commandInfo->element;
	}
	ASSIGN_RESULT(NULL, false, true);
}
