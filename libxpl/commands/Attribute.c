#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdAttributeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdAttributeParams
{
	xplQName qname;
	xmlXPathObjectPtr destination;
	bool replace;
	bool force_blank;
} xplCmdAttributeParams, *xplCmdAttributeParamsPtr;

static const xplCmdAttributeParams params_stencil =
{
	.qname = {},
	.destination = NULL,
	.replace = true,
	.force_blank = false
};

xplCommand xplAttributeCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdAttributeEpilogue,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdAttributeParams),
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE,
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.required = true,
			.value_stencil = &params_stencil.qname
		}, {
			.name = BAD_CAST "destination",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_NODESET,
			.value_stencil = &params_stencil.destination
		}, {
			.name = BAD_CAST "replace",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.replace
		}, {
			.name = BAD_CAST "forceblank",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.force_blank
		}, {
			.name = NULL
		}
	}
};

void xplCmdAttributeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdAttributeParamsPtr params = (xplCmdAttributeParamsPtr) commandInfo->params;
	xmlChar *attr_value;
	xmlNodePtr cur;
	size_t i;
	
	if (!commandInfo->content || !*commandInfo->content)
	{
		if (!params->force_blank)
		{
			ASSIGN_RESULT(NULL, false, true);
			return; /* empty value, nothing to do */
		} else
			attr_value = BAD_CAST "";
	} else
		attr_value = commandInfo->content;

	if (params->destination && params->destination->nodesetval)
	{
		for (i = 0; i < (size_t) params->destination->nodesetval->nodeNr; i++)
		{
			cur = params->destination->nodesetval->nodeTab[i];
			if (cur->type == XML_ELEMENT_NODE)
				(void) xplCreateAttribute(cur, params->qname, attr_value, params->replace);
			else if (cfgWarnOnInvalidNodeType)
				xplDisplayMessage(XPL_MSG_WARNING, BAD_CAST "xpl:attribute: can't assign attributes to non-elements, file '%s', line %d, destination '%s'",
				commandInfo->element->doc->URL, commandInfo->element->line, params->destination->user);
		}
	} else if (commandInfo->element->parent) 
		(void) xplCreateAttribute(commandInfo->element->parent, params->qname, attr_value, params->replace);
	ASSIGN_RESULT(NULL, false, true);
}
