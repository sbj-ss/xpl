#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>
#include "commands/Edge.h"

typedef enum {
	EDGE_COPY,
	EDGE_REPLACE,
	EDGE_ELEMENT,
	EDGE_ATTRIBUTE,
	EDGE_ERROR = -1
} EdgeType;

EdgeType edgeTypeFromString(xmlChar *s)
{
	if (!xmlStrcasecmp(s, BAD_CAST "copy"))
		return EDGE_COPY;
	else if (!xmlStrcasecmp(s, BAD_CAST "replace"))
		return EDGE_REPLACE;
	else if (!xmlStrcasecmp(s, BAD_CAST "element"))
		return EDGE_ELEMENT;
	else if (!xmlStrcasecmp(s, BAD_CAST "attribute"))
		return EDGE_ATTRIBUTE;
	return EDGE_ERROR;
}

xmlNodePtr cloneNodeSet(xmlNodeSetPtr set, xmlNodePtr parent, xmlNodePtr *tail)
{
	size_t i;
	xmlNodePtr ret = NULL, cur, tail_int = NULL;
	for (i = 0; i < (size_t) set->nodeNr; i++)
	{
		cur = xplCloneAttrAsText(set->nodeTab[i], parent);
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

xmlChar* flattenTextSet(xmlNodeSetPtr set)
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

void xplCmdEdgePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdEdgeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")
#define TYPE_ATTR (BAD_CAST "type")
#define SOURCE_ATTR (BAD_CAST "source")
#define DESTINATION_ATTR (BAD_CAST "destination")
	xmlChar *name_attr = NULL;
	xmlChar *type_attr = NULL;
	xmlChar *source_attr = NULL;
	xmlChar *destination_attr = NULL;
	EdgeType type = EDGE_COPY;
	xmlXPathObjectPtr src = NULL, dst = NULL;
	xmlNodeSetPtr dst_nodes = NULL;
	xmlNodePtr source_list = NULL, cur, el;
	xmlChar *source_text= NULL;
	xmlNsPtr ns = NULL, cur_ns;
	xmlChar *tagname;
	size_t i;

	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	type_attr = xmlGetNoNsProp(commandInfo->element, TYPE_ATTR);
	source_attr = xmlGetNoNsProp(commandInfo->element, SOURCE_ATTR);
	destination_attr = xmlGetNoNsProp(commandInfo->element, DESTINATION_ATTR);

	if (type_attr)
	{
		if ((type = edgeTypeFromString(type_attr)) == EDGE_ERROR)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid type argument: \"%s\"", type_attr), true, true);
			goto done;
		}
	}
	/* validity checks */
	if (((type == EDGE_ELEMENT) || (type == EDGE_ATTRIBUTE))) 
	{
		if (!name_attr)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "missing name argument"), true, true);
			goto done;
		}
		EXTRACT_NS_AND_TAGNAME(name_attr, ns, tagname, commandInfo->element);
		if (xmlValidateName(tagname, 0)) 
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid name argument: \"%s\"", name_attr), true, true);
			goto done;
		}
	} else {
		if (name_attr)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "using name argument for this operation type makes no sense"), true, true);
			goto done;
		}
		/* it's better to allow source and destination to be missing simultaneously */
		/* we'll have a simple container this way */
#if 0
		if (!source_attr && !destination_attr)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo, BAD_CAST "at least source or destination must be specified for this operation type"), true, true);
			goto done;
		}
#endif
	}
	/* obtain source data taking into account they can be text only */
	if (source_attr)
	{
		src = xplSelectNodes(commandInfo->document, commandInfo->element, source_attr);
		if (!src)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid XPath source expression \"%s\"", source_attr), true, true);
			goto done;
		}
		if (type == EDGE_ATTRIBUTE) 
		{
			switch (src->type)
			{
			case XPATH_NODESET:
				if (!src->nodesetval)
				{
					ASSIGN_RESULT(NULL, false, true);
					goto done;
				}
				if (!xplCheckNodeSetForText(src->nodesetval))
				{
					ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "non-text nodes inside"), true, true);
					goto done;
				}
				source_text = flattenTextSet(src->nodesetval);
				break;
			case XPATH_BOOLEAN:
			case XPATH_NUMBER:
			case XPATH_STRING:
				source_text = xmlXPathCastToString(src);
				break;
			default:
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "source XPath expression \"%s\" evaluated to an invalid result", source_attr), true, true);
				goto done;
			}
		} else
			source_list = cloneNodeSet(src->nodesetval, commandInfo->element, NULL);
	} else {
		if (type == EDGE_ATTRIBUTE)
		{
			if (!xplCheckNodeListForText(commandInfo->element->children))
			{
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "non-text nodes inside"), true, true);
					goto done;
			}
			source_text = xmlNodeListGetString(commandInfo->element->doc, commandInfo->element->children, 1);
			if (!source_text)
			{
				ASSIGN_RESULT(NULL, false, true); /* nothing to do */
				goto done;
			}
		} else
			source_list = commandInfo->element->children;
	}
	/* determine destination(s) */
	if (destination_attr)
	{
		dst = xplSelectNodes(commandInfo->document, commandInfo->element, destination_attr);
		if (!dst)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid XPath destination expression \"%s\"", destination_attr), true, true);
			goto done;
		}
		if (dst->type != XPATH_NODESET)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "destination XPath expression \"%s\" evaluated to non-nodeset value", destination_attr), true, true);
			goto done;
		}
		dst_nodes = dst->nodesetval;
	} else
		dst_nodes = xmlXPathNodeSetCreate(commandInfo->element->parent);
	/* execute the requested operation */
	for (i = 0; i < (size_t) dst_nodes->nodeNr; i++)
	{
		cur = dst_nodes->nodeTab[i];
		switch (type)
		{
		case EDGE_COPY:
			if (cur->type == XML_ELEMENT_NODE)
				xplAppendChildren(cur, xplCloneNodeList(source_list, cur, cur->doc));
			break;
		case EDGE_REPLACE:
			if (!xplIsAncestor(commandInfo->element, cur))
				xplDocDeferNodeDeletion(commandInfo->document, xplReplaceWithList(cur, xplCloneNodeList(source_list, cur, cur->doc)));
			else if (cfgWarnOnAncestorModificationAttempt)
			{
				if (cur->ns)
					xplDisplayMessage(xplMsgWarning, BAD_CAST "ancestor/self node \"%s:%s\" modification attempt denied (file \"%s\", line %d)", 
					cur->ns->prefix, cur->name, commandInfo->document->document->URL, commandInfo->element->line);
				else
					xplDisplayMessage(xplMsgWarning, BAD_CAST "ancestor/self node \"%s\" modification attempt denied (file \"%s\", line %d)", 
					cur->name, commandInfo->document->document->URL, commandInfo->element->line);
			}
			break;
		case EDGE_ELEMENT:
			if (cur->type == XML_ELEMENT_NODE)
			{
				cur_ns = ns? xplGetResultingNs(cur, commandInfo->element, BAD_CAST ns->prefix): NULL;
				el = xmlNewDocNode(cur->doc, cur_ns, tagname, NULL);
				xplAppendChildren(cur, el);
				xplSetChildren(el, xplCloneNodeList(source_list, el, el->doc));
			}
			break;
		case EDGE_ATTRIBUTE:
			if (cur->type == XML_ELEMENT_NODE)
			{
				cur_ns = ns? xplGetResultingNs(cur, commandInfo->element, BAD_CAST ns->prefix): NULL;
				if (cur_ns)
					xmlNewNsProp(cur, cur_ns, tagname, source_text);
				else
					xmlNewProp(cur, tagname, source_text);
			}
			break;
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
		}
	}
	ASSIGN_RESULT(NULL, false, true);
done:
	if (name_attr)
		XPL_FREE(name_attr);
	if (type_attr)
		XPL_FREE(type_attr);
	if (source_attr)
		XPL_FREE(source_attr);
	if (destination_attr)
		XPL_FREE(destination_attr);
	if (src) 
	{	
		if (src->nodesetval)
			src->nodesetval->nodeNr = 0;
		xmlXPathFreeObject(src);
		if (source_list)
			xmlFreeNodeList(source_list);
	}
	if (source_text)
		XPL_FREE(source_text);
	if (dst) 
	{
		if (dst->nodesetval)
			dst->nodesetval->nodeNr = 0;
		xmlXPathFreeObject(dst);
	}
	else if (dst_nodes)
		xmlXPathFreeNodeSet(dst_nodes);
}

xplCommand xplEdgeCommand = { xplCmdEdgePrologue, xplCmdEdgeEpilogue };
