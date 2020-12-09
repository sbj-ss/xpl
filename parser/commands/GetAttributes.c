#include <string.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include "commands/GetAttributes.h"

typedef struct _xplCmdGetAttributesParams
{
	xmlXPathObjectPtr select;
	xplQName tagname;
	bool show_tags;
	bool repeat;
} xplCmdGetAttributesParams, *xplCmdGetAttributesParamsPtr;

static const xplCmdGetAttributesParams params_stencil =
{
	.select = NULL,
	.tagname = { NULL, BAD_CAST "attribute" },
	.show_tags = false,
	.repeat = true
};

xplCommand xplGetAttributesCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdGetAttributesEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdGetAttributesParams),
	.parameters = {
		{
			.name = BAD_CAST "select",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.extra = {
				.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_NODESET
			},
			.value_stencil = &params_stencil.select
		}, {
			.name = BAD_CAST "tagname",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.value_stencil = &params_stencil.tagname
		}, {
			.name = BAD_CAST "showtags",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.show_tags
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};

static xmlNodePtr _wrapAttributes(xmlAttrPtr prop, xplQName tagname, bool show_tags)
{
	xmlNodePtr ret = NULL, tail, cur;
	xmlChar *name;
	xmlAttrPtr name_prop;
	xmlNsPtr ns;

	while (prop)
	{
		if (show_tags)
		{
			cur = xmlNewDocNode(prop->doc, prop->ns, prop->name, NULL);
			if (prop->ns)
			{
				ns = xmlSearchNsByHref(prop->doc, prop->parent->parent, prop->ns->href);
				if (!ns)
					cur->ns = xmlNewNs(cur, prop->ns->href, prop->ns->prefix);
			}
		} else {
			cur = xmlNewDocNode(prop->doc, tagname.ns, tagname.ncname, NULL);
			if (prop->ns && prop->ns->prefix)
			{
				name = (xmlChar*) XPL_MALLOC((size_t) xmlStrlen(prop->ns->prefix) + xmlStrlen(prop->name) + 2);
				strcpy((char*) name, (char*) prop->ns->prefix);
				strcat((char*) name, ":");
				strcat((char*) name, (char*) prop->name);
			} else
				name = BAD_CAST XPL_STRDUP((char*) prop->name);
			name_prop = xmlNewProp(cur, BAD_CAST "name", NULL);
			name_prop->children = xmlNewDocText(prop->doc, NULL);
			name_prop->children->content = name;
		}
		cur->children = xmlNewDocText(prop->doc, NULL);
		cur->children->content = xplGetPropValue(prop);
		if (!ret)
			ret = tail = cur;
		else {
			tail->next = cur;
			cur->prev = tail;
			tail = cur;
		}
		prop = prop->next;
	}
	return ret;
}

void xplCmdGetAttributesEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdGetAttributesParamsPtr params = (xplCmdGetAttributesParamsPtr) commandInfo->params;
	xmlNodePtr src = NULL;

	if (params->select)
	{
		if (params->select->nodesetval)
		{
			if ((params->select->nodesetval->nodeNr == 1) && (params->select->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE))
				src = params->select->nodesetval->nodeTab[0];
			else if (!params->select->nodesetval->nodeNr) {
				ASSIGN_RESULT(NULL, false, true);
				return;
			} else {
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath (%s) must evaluate to a single element node", params->select->user), true, true);
				return;
			}
		}
	} else {
		src = commandInfo->element->children;
		while (src && (src->type != XML_ELEMENT_NODE))
			src = src->next;
		if (!src)
		{
			ASSIGN_RESULT(NULL, false, true);
			return;
		}
	}

	ASSIGN_RESULT(_wrapAttributes(src->properties, params->tagname, params->show_tags), params->repeat, true);
}
