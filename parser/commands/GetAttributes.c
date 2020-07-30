#include <string.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include "commands/GetAttributes.h"

void xplCmdGetAttributesPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdGetAttributesEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define SELECT_ATTR (BAD_CAST "select")
#define TAGNAME_ATTR (BAD_CAST "tagname")
#define SHOWTAGS_ATTR (BAD_CAST "showtags")
#define DEFAULT_TAG_NAME (BAD_CAST "attribute")
#define REPEAT_ATTR (BAD_CAST "repeat")
	xmlChar *select_attr = NULL;
	xmlChar *tagname_attr = NULL;
	bool showtags;
	bool repeat;
	xmlNodePtr src = NULL;
	xmlNodePtr ret = NULL, tail, cur, error;
	xmlChar *tagname, *name;
	xmlXPathObjectPtr sel = NULL;
	xmlNsPtr ns;
	xmlAttrPtr prop, temp_prop;

	select_attr = xmlGetNoNsProp(commandInfo->element, SELECT_ATTR);
	tagname_attr = xmlGetNoNsProp(commandInfo->element, TAGNAME_ATTR);
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, SHOWTAGS_ATTR, &showtags, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, true)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}

	if (!tagname_attr)
	{
		tagname = DEFAULT_TAG_NAME;
		ns = NULL;
	} else {
		EXTRACT_NS_AND_TAGNAME(tagname_attr, ns, tagname, commandInfo->element)
	}

	if (select_attr)
	{
		sel = xplSelectNodes(commandInfo->document, commandInfo->element, select_attr);
		if (sel)
		{
			if (sel->type == XPATH_NODESET)
			{
				if (sel->nodesetval)
				{
					if ((sel->nodesetval->nodeNr == 1) && (sel->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE))
						src = sel->nodesetval->nodeTab[0];
					else if (!sel->nodesetval->nodeNr) {
						ASSIGN_RESULT(NULL, false, true);
						goto done;
					} else {
						ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath (%s) must evaluate to a single element node", select_attr), true, true);
						goto done;
					}
				} else
					goto done;
			} else {
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath (%s) evaluated to non-nodeset value", select_attr), true, true);
				goto done;
			}
		} else {
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid select XPath expression (%s)", select_attr), true, true);
			goto done;
		}
	} else {
		src = commandInfo->element->children;
		while (src && (src->type != XML_ELEMENT_NODE))
			src = src->next;
		if (!src)
		{
			ASSIGN_RESULT(NULL, false, true);
			goto done;
		}
	}

	prop = src->properties;
	while (prop)
	{
		if (showtags)
		{
			cur = xmlNewDocNode(commandInfo->element->doc, prop->ns, prop->name, NULL);
			if (prop->ns)
			{
				ns = xmlSearchNsByHref(commandInfo->element->doc, commandInfo->element->parent, prop->ns->href);
				if (!ns)
					cur->ns = xmlNewNs(cur, prop->ns->href, prop->ns->prefix);
			}
		} else {
			cur = xmlNewDocNode(commandInfo->element->doc, ns, tagname, NULL);
			if (prop->ns && prop->ns->prefix)
			{
				name = (xmlChar*) XPL_MALLOC((size_t) xmlStrlen(prop->ns->prefix) + xmlStrlen(prop->name) + 2);
				strcpy((char*) name, (char*) prop->ns->prefix);
				strcat((char*) name, ":");
				strcat((char*) name, (char*) prop->name);
			} else
				name = BAD_CAST XPL_STRDUP((char*) prop->name);
			temp_prop = xmlNewProp(cur, BAD_CAST "name", NULL);
			temp_prop->children = xmlNewDocText(commandInfo->element->doc, NULL);
			temp_prop->children->content = name;
		}
		cur->children = xmlNewDocText(commandInfo->element->doc, NULL);
		cur->children->content = getPropValue(prop);
		if (!ret)
			ret = tail = cur;
		else {
			tail->next = cur;
			cur->prev = tail;
			tail = cur;
		}
		prop = prop->next;
	}
	ASSIGN_RESULT(ret, repeat, true);
done:
	if (select_attr)
		XPL_FREE(select_attr);
	if (tagname_attr)
		XPL_FREE(tagname_attr);
	if (sel)
		xmlXPathFreeObject(sel);
}

xplCommand xplGetAttributesCommand = { xplCmdGetAttributesPrologue, xplCmdGetAttributesEpilogue };
