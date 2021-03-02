#include <string.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
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

static xmlNodePtr _wrapAttributes(const xmlAttrPtr prop, const xplQName tagname)
{
	xmlNodePtr ret = NULL, tail = NULL, cur;
	xmlChar *name;
	xmlAttrPtr cur_prop = prop, name_prop;
	xmlNsPtr ns;

	while (cur_prop)
	{
		if (!tagname.ncname)
		{
			cur = xmlNewDocNode(cur_prop->doc, cur_prop->ns, cur_prop->name, NULL);
			if (cur_prop->ns)
			{
				ns = xmlSearchNsByHref(cur_prop->doc, cur_prop->parent->parent, cur_prop->ns->href);
				if (!ns)
					cur->ns = cur->nsDef = xmlNewNs(cur, cur_prop->ns->href, cur_prop->ns->prefix);
			}
		} else {
			cur = xmlNewDocNode(cur_prop->doc, tagname.ns, tagname.ncname, NULL);
			if (cur_prop->ns && cur_prop->ns->prefix)
			{
				name = (xmlChar*) XPL_MALLOC((size_t) xmlStrlen(cur_prop->ns->prefix) + xmlStrlen(cur_prop->name) + 2);
				strcpy((char*) name, (char*) cur_prop->ns->prefix);
				strcat((char*) name, ":");
				strcat((char*) name, (char*) cur_prop->name);
			} else
				name = BAD_CAST XPL_STRDUP((char*) cur_prop->name);
			name_prop = xmlNewProp(cur, BAD_CAST "name", NULL);
			name_prop->children = name_prop->last = xmlNewDocText(cur_prop->doc, NULL);
			name_prop->children->parent = (xmlNodePtr) name_prop;
			name_prop->children->content = name;
		}
		cur->children = cur->last = xmlNewDocText(cur_prop->doc, NULL);
		cur->children->parent = cur;
		cur->children->content = xplGetPropValue(cur_prop);
		APPEND_NODE_TO_LIST(ret, tail, cur);
		cur_prop = cur_prop->next;
	}
	return ret;
}

static bool _hasElementTail(const xmlNodePtr maybeTail)
{
	xmlNodePtr cur = maybeTail;

	while (cur)
	{
		if (cur->type == XML_ELEMENT_NODE)
			return true;
		cur = cur->next;
	}
	return false;
}

static bool _hasMoreElements(const xmlNodeSetPtr nodes, ssize_t i)
{
	while (i < nodes->nodeNr)
	{
		if (nodes->nodeTab[i]->type == XML_ELEMENT_NODE)
			return true;
		i++;
	}
	return false;
}

static xplQName empty_qname = { NULL, NULL };
static xplQName default_tag_name = { NULL, BAD_CAST "attribute" };

void xplCmdGetAttributesEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdGetAttributesParamsPtr params = (xplCmdGetAttributesParamsPtr) commandInfo->params;
	xmlNodePtr src = NULL;
	xplQName tag_name;
	ssize_t i = 0;
	xmlNodeSetPtr nodes;

	if (params->tagname.ncname && params->show_tags)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "showtags and tagname can't be used simultaneously"), true, true);
		return;
	}

	if (params->select && (nodes = params->select->nodesetval) && nodes->nodeNr)
	{
		while ((i < nodes->nodeNr) && (nodes->nodeTab[i]->type != XML_ELEMENT_NODE))
			i++;
		if (cfgWarnOnInvalidNodeType && i)
			xplDisplayWarning(commandInfo->element, BAD_CAST "can only work with element nodes, select '%s'", params->select->user);
		if (i < nodes->nodeNr)
		{
			src = params->select->nodesetval->nodeTab[i];
			if (cfgWarnOnMultipleSelection && _hasMoreElements(params->select->nodesetval, i))
				xplDisplayWarning(commandInfo->element, BAD_CAST "gets attributes only from the first selected element but more elements follow, select '%s'", params->select->user);
		}
	} else if (!params->select) {
		if (!(src = xplFirstElementNode(commandInfo->element->children)))
		{
			ASSIGN_RESULT(NULL, false, true);
			return;
		}
		if (cfgWarnOnInvalidNodeType && src != commandInfo->element->children)
			xplDisplayWarning(commandInfo->element, BAD_CAST "can only work with element nodes, select '%s'", params->select->user);
		if (cfgWarnOnMultipleSelection && _hasElementTail(src))
			xplDisplayWarning(commandInfo->element, BAD_CAST "gets attributes only from its first child element but more elements follow");
	}
	if (!src)
	{
		ASSIGN_RESULT(NULL, false, true);
		return;
	}
	tag_name = params->show_tags? empty_qname: params->tagname.ncname? params->tagname: default_tag_name;
	ASSIGN_RESULT(_wrapAttributes(src->properties, tag_name), params->repeat, true);
}
