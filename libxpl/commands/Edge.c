#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdEdgeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef enum {
	EDGE_COPY,
	EDGE_REPLACE,
	EDGE_ELEMENT,
	EDGE_ATTRIBUTE
} EdgeType;

typedef struct _xplCmdEdgeParams
{
	xplQName name;
	EdgeType type;
	xmlXPathObjectPtr source;
	xmlXPathObjectPtr destination;
} xplCmdEdgeParams, *xplCmdEdgeParamsPtr;

static const xplCmdEdgeParams params_stencil =
{
	.name = { NULL, NULL },
	.type = EDGE_COPY,
	.source = NULL,
	.destination = NULL
};

static xplCmdParamDictValue type_values[] =
{
	{ .name = BAD_CAST "copy", .value = EDGE_COPY },
	{ .name = BAD_CAST "replace", .value = EDGE_REPLACE },
	{ .name = BAD_CAST "element", .value = EDGE_ELEMENT },
	{ .name = BAD_CAST "attribute", .value = EDGE_ATTRIBUTE },
	{ .name = NULL }
};

xplCommand xplEdgeCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdEdgeEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdEdgeParams),
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.value_stencil = &params_stencil.name
		}, {
			.name = BAD_CAST "type",
			.type = XPL_CMD_PARAM_TYPE_DICT,
			.extra.dict_values = type_values,
			.value_stencil = &params_stencil.type
		}, {
			.name = BAD_CAST "source",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_ANY,
			.value_stencil = &params_stencil.source
		}, {
			.name = BAD_CAST "destination",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_NODESET,
			.value_stencil = &params_stencil.destination
		}, {
			.name = NULL
		}
	}
};

static xmlNodePtr _cloneNodeSet(xmlNodeSetPtr set, xmlNodePtr parent, xmlNodePtr *tail)
{
	size_t i;
	xmlNodePtr ret = NULL, cur, tail_int = NULL;
	for (i = 0; i < (size_t) set->nodeNr; i++)
	{
		cur = xplCloneAsNodeChild(set->nodeTab[i], parent);
		if (ret)
		{
			tail_int->next = cur;
			cur->prev = tail_int;
			tail_int = cur;
		} else
			ret = tail_int = cur;
		tail_int = xplFindTail(tail_int);
	}
	if (tail)
		*tail = tail_int;
	return ret;
}

static xmlChar* _flattenTextSet(xmlNodeSetPtr set)
{
	xmlChar *ret = NULL, *txt;
	xmlNodePtr cur, tmp;
	size_t i;

	for (i = 0; i < (size_t) set->nodeNr; i++)
	{
		if (set->nodeTab[i]->type == XML_TEXT_NODE)
		{
			cur = set->nodeTab[i];
			while (cur->next && ((cur->next->type == XML_TEXT_NODE) 
				|| (cur->next->type == XML_ENTITY_REF_NODE)
				|| (cur->next->type == XML_CDATA_SECTION_NODE))
			)
				cur = cur->next;
			tmp = cur->next;
			cur->next = NULL;
			txt = xmlNodeListGetString(set->nodeTab[i]->doc, set->nodeTab[i], 1);
			cur->next = tmp;
		} else
			txt = xmlNodeListGetString(set->nodeTab[i]->doc, set->nodeTab[i]->children, 1);
		if (txt)
		{
			ret = xmlStrcat(ret, txt);
			XPL_FREE(txt);
		}
	}
	return ret;
}

static xmlNsPtr _getResultingNs(xmlNodePtr cur, xmlNsPtr ns)
{
	xmlNsPtr ret;

	if (!ns)
		return NULL;
	if ((ret = xmlSearchNsByHref(cur->doc, cur, ns->href)))
		return ret;
	return xmlNewNs(cur, ns->href, ns->prefix);
}

void xplCmdEdgeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdEdgeParamsPtr params = (xplCmdEdgeParamsPtr) commandInfo->params;
	xmlNodeSetPtr dst_nodes = NULL;
	xmlNodePtr source_list = NULL, cur, el;
	xmlChar *source_text = NULL;
	xmlNsPtr cur_ns;
	size_t i;

	/* validity checks */
	if (params->type == EDGE_ELEMENT || params->type == EDGE_ATTRIBUTE)
	{
		if (!params->name.ncname)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "missing name argument"), true, true);
			return;
		}
	} else if (params->name.ncname)	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "using name argument for this operation type makes no sense"), true, true);
		return;
	}
	/* obtain source data taking into account they can be text only */
	if (params->source)
	{
		if (params->type == EDGE_ATTRIBUTE)
		{
			switch (params->source->type)
			{
				case XPATH_NODESET:
					if (!params->source->nodesetval)
					{
						ASSIGN_RESULT(NULL, false, true);
						return;
					}
					if (!xplCheckNodeSetForText(params->source->nodesetval))
					{
						ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "can't create attribute from non-text nodes"), true, true);
						return;
					}
					source_text = _flattenTextSet(params->source->nodesetval);
					break;
				case XPATH_BOOLEAN:
				case XPATH_NUMBER:
				case XPATH_STRING:
					source_text = xmlXPathCastToString(params->source);
					break;
				default:
					ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "source XPath expression '%s' evaluated to an invalid result", params->source->user), true, true);
					return;
			}
		} else
			source_list = _cloneNodeSet(params->source->nodesetval, commandInfo->element, NULL); // TODO do we really need a copy?
	} else {
		if (params->type == EDGE_ATTRIBUTE)
		{
			if (!xplCheckNodeListForText(commandInfo->element->children))
			{
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "non-text nodes inside"), true, true);
				return;
			}
			source_text = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, 1);
			if (!source_text)
			{
				ASSIGN_RESULT(NULL, false, true); /* nothing to do */
				return;
			}
		} else
			source_list = commandInfo->element->children;
	}
	/* determine destination(s) */
	if (params->destination)
		dst_nodes = params->destination->nodesetval;
	else
		dst_nodes = xmlXPathNodeSetCreate(commandInfo->element->parent);
	/* execute the requested operation */
	for (i = 0; i < (size_t) dst_nodes->nodeNr; i++)
	{
		cur = dst_nodes->nodeTab[i];
		switch (params->type)
		{
			case EDGE_COPY:
				if (cur->type == XML_ELEMENT_NODE)
					xplAppendChildren(cur, xplCloneNodeList(source_list, cur, cur->doc));
				break;
			case EDGE_REPLACE:
				if ((commandInfo->element != cur) && !xplIsAncestor(commandInfo->element, cur))
					xplDocDeferNodeDeletion(commandInfo->document, xplReplaceWithList(cur, xplCloneNodeList(source_list, cur, cur->doc)));
				else if (cfgWarnOnAncestorModificationAttempt)
				{
					if (cur->ns)
						xplDisplayMessage(XPL_MSG_WARNING, BAD_CAST "ancestor/self node \"%s:%s\" modification attempt denied (file \"%s\", line %d)",
						cur->ns->prefix, cur->name, commandInfo->document->document->URL, commandInfo->element->line);
					else
						xplDisplayMessage(XPL_MSG_WARNING, BAD_CAST "ancestor/self node \"%s\" modification attempt denied (file \"%s\", line %d)",
						cur->name, commandInfo->document->document->URL, commandInfo->element->line);
				}
				break;
			case EDGE_ELEMENT:
				if (cur->type == XML_ELEMENT_NODE)
				{
					cur_ns = _getResultingNs(cur, params->name.ns);
					el = xmlNewDocNode(cur->doc, cur_ns, params->name.ncname, NULL);
					xplAppendChildren(cur, el);
					xplSetChildren(el, xplCloneNodeList(source_list, el, el->doc));
				} // TODO warn otherwise
				break;
			case EDGE_ATTRIBUTE:
				if (cur->type == XML_ELEMENT_NODE)
				{
					cur_ns = _getResultingNs(cur, params->name.ns);
					if (cur_ns)
						xmlNewNsProp(cur, cur_ns, params->name.ncname, source_text);
					else
						xmlNewProp(cur, params->name.ncname, source_text);
				} // TODO warn otherwise
				break;
			default:
				DISPLAY_INTERNAL_ERROR_MESSAGE();
		}
	}
	ASSIGN_RESULT(NULL, false, true);

	if (source_list && (source_list != commandInfo->element->children))
		xmlFreeNodeList(source_list);
	if (source_text)
		XPL_FREE(source_text);
	if (dst_nodes && (!params->destination || (dst_nodes != params->destination->nodesetval)))
		xmlXPathFreeNodeSet(dst_nodes);
}
