#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>
#include "commands/Break.h"

void xplCmdBreakPrologue(xplCommandInfoPtr commandInfo)
{
}

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

#define POINT_ATTR (BAD_CAST "point")
	xmlChar *point_attr = NULL;
	xmlXPathObjectPtr point = NULL;
	xmlChar number_buf[32];
	bool do_break = true;
	bool do_climb = false;
	xmlChar *upper_point;
	xmlNodePtr upper_break;

	point_attr = xmlGetNoNsProp(commandInfo->element, POINT_ATTR);
	if (point_attr) /* ������� ����� */
	{
		point = xplSelectNodes(commandInfo->document, commandInfo->element, point_attr);
		if (!point)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid point XPath expression \"%s\"", point_attr), true, true);
			goto done;
		}
		switch (point->type)
		{
		case XPATH_BOOLEAN: 
			do_break = point->boolval;
			break;
		case XPATH_STRING:
			do_break = (point->stringval && *point->stringval);
			break;
		case XPATH_NUMBER:
			do_climb = (point->floatval > 1.0);
			if (do_climb)
			{
				snprintf((char*) number_buf, sizeof(number_buf) - 1, "%d", (int) point->floatval - 1);
				number_buf[sizeof(number_buf) - 1] = 0;
				upper_point = number_buf;
			}
			break;
		case XPATH_NODESET:
			if (point->nodesetval && point->nodesetval->nodeNr)
				do_climb = !checkForAncestor(point->nodesetval, commandInfo->element->parent);
			else
				do_climb = true;
			upper_point = point_attr;
			break;
		default:
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "point XPath expression \"%s\" evaluated to invalid type", point_attr), true, true);
			goto done;
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
	if (do_break)
	{
		xplDocDeferNodeListDeletion(commandInfo->document, commandInfo->element->next);
		commandInfo->element->next = NULL;
		commandInfo->element->parent->last = commandInfo->element;
	}
	ASSIGN_RESULT(NULL, false, true);
done:
	if (point) xmlXPathFreeObject(point);
	if (point_attr) xmlFree(point_attr);
}

xplCommand xplBreakCommand = { xplCmdBreakPrologue, xplCmdBreakEpilogue };
