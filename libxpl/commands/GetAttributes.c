#include <string.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdGetAttributesEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

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
	.tagname = { NULL, NULL },
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

static xmlNodePtr _wrapAttributes(xmlAttrPtr prop, xplQName tagname)
{
	xmlNodePtr ret = NULL, tail, cur;
	xmlChar *name;
	xmlAttrPtr name_prop;
	xmlNsPtr ns;

	while (prop)
	{
		if (!tagname.ncname)
		{
			cur = xmlNewDocNode(prop->doc, prop->ns, prop->name, NULL);
			if (prop->ns)
			{
				ns = xmlSearchNsByHref(prop->doc, prop->parent->parent, prop->ns->href);
				if (!ns)
					cur->ns = cur->nsDef = xmlNewNs(cur, prop->ns->href, prop->ns->prefix);
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
			name_prop->children = name_prop->last = xmlNewDocText(prop->doc, NULL);
			name_prop->children->parent = (xmlNodePtr) name_prop;
			name_prop->children->content = name;
		}
		cur->children = cur->last = xmlNewDocText(prop->doc, NULL);
		cur->children->parent = cur;
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

static xplQName empty_qname = { NULL, NULL };
static xplQName default_tag_name = { NULL, BAD_CAST "attribute" };

void xplCmdGetAttributesEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdGetAttributesParamsPtr params = (xplCmdGetAttributesParamsPtr) commandInfo->params;
	xmlNodePtr src = NULL;
	xplQName tag_name;

	if (params->tagname.ncname && params->show_tags)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "showtags and tagname can't be used simultaneously"), true, true);
		return;
	}

	if (params->select)
	{
		if (params->select->nodesetval)
		{
			if ((params->select->nodesetval->nodeNr == 1) && (params->select->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE))
			{
				src = params->select->nodesetval->nodeTab[0];
				if (src->type != XML_ELEMENT_NODE)
				{
					ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath (%s) evaluated to non-element node(s)"), true, true);
					return;
				}
			} else if (!params->select->nodesetval->nodeNr) {
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
		} // TODO warning if more elements follow
	}
	tag_name = params->show_tags? empty_qname: params->tagname.ncname? params->tagname: default_tag_name;
	ASSIGN_RESULT(_wrapAttributes(src->properties, tag_name), params->repeat, true);
}
