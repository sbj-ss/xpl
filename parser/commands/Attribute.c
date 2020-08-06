#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include "commands/Attribute.h"

static void _assignAttribute(xmlNodePtr dst, xplQNamePtr qname, xmlChar *value, bool allowReplace)
{
	xmlAttrPtr prev;

	if (qname->ns)
		prev = xmlHasNsProp(dst, qname->ncname, qname->ns->href);
	else
		prev = xmlHasProp(dst, qname->ncname);
	if (prev)
	{
		if (!allowReplace)
			return;
		xmlFreeNodeList(prev->children);
		xplSetChildren((xmlNodePtr) prev, xmlNewDocText(dst->doc, value));
	} else {
		if (qname->ns)
			xmlNewNsProp(dst, qname->ns, qname->ncname, value);
		else
			xmlNewProp(dst, qname->ncname, value);
	}
}

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
	.prologue = xplCmdAttributePrologue,
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

void xplCmdAttributePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdAttributeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdAttributeParamsPtr cmd_params = (xplCmdAttributeParamsPtr) commandInfo->params;
	xmlChar *attr_value;
	xmlNodePtr cur;
	size_t i;
	
	if (!commandInfo->content)
	{
		if (!cmd_params->force_blank)
		{
			ASSIGN_RESULT(NULL, false, true);
			return; /* empty value, nothing to do */
		} else
			attr_value = BAD_CAST "";
	} else
		attr_value = commandInfo->content;

	if (cmd_params->destination && cmd_params->destination->nodesetval)
	{
		for (i = 0; i < (size_t) cmd_params->destination->nodesetval->nodeNr; i++)
		{
			cur = cmd_params->destination->nodesetval->nodeTab[i];
			if (cur->type == XML_ELEMENT_NODE)
				_assignAttribute(cur, &cmd_params->qname, attr_value, cmd_params->replace);
		}
	} else if (commandInfo->element->parent) 
		_assignAttribute(commandInfo->element->parent, &cmd_params->qname, attr_value, cmd_params->replace);
		ASSIGN_RESULT(NULL, false, true);
}
