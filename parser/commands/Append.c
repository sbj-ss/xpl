#include "commands/Append.h"
#include "Core.h"
#include "Messages.h"
#include "Utils.h"

typedef enum {
	POS_BEFORE,
	POS_AFTER,
	POS_FIRST,
	POS_LAST,
	POS_ERROR = -1
} AppendMode;

AppendMode appendModeFromString(xmlChar *s)
{
	if (!xmlStrcasecmp(s, BAD_CAST "before"))
		return POS_BEFORE;
	else if (!xmlStrcasecmp(s, BAD_CAST "after"))
		return POS_AFTER;
	else if (!xmlStrcasecmp(s, BAD_CAST "first"))
		return POS_FIRST;
	else if (!xmlStrcasecmp(s, BAD_CAST "last"))
		return POS_LAST;
	return POS_ERROR;
}

void doAppend(xmlNodePtr src, xmlNodePtr dst, AppendMode mode)
{
	switch (mode)
	{
		case POS_BEFORE: 
			prependList(dst, src);
			break;
		case POS_AFTER:
			appendList(dst, src);
			break;
		case POS_FIRST:
			if (dst->children)
				prependList(dst->children, src);
			else {
				dst->children = src;
				dst->last = findTail(src);
				while (src)
				{
					src->parent = dst;
					src = src->next;
				}
			}
			break;
		case POS_LAST:
			if (dst->children)
				appendList(dst->last?dst->last:findTail(dst->children), src);
			else {
				dst->children = src;
				dst->last = findTail(src);
				while (src)
				{
					src->parent = dst;
					src = src->next;
				}
			}
			break;
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
	}
}

void xplCmdAppendPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdAppendEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define SELECT_ATTR (BAD_CAST "select")
#define DESTINATION_ATTR (BAD_CAST "destination")
#define SOURCE_ATTR (BAD_CAST "source")
#define POSITION_ATTR (BAD_CAST "position")

#define PARENT(node) ((position == POS_AFTER) || (position == POS_BEFORE))?((node)->parent):node

	xmlChar* position_attr = NULL;
	xmlChar *select_attr = NULL;
	xmlChar *source_attr = NULL;
	AppendMode position = POS_LAST;
	xmlXPathObjectPtr src = NULL;
	xmlXPathObjectPtr dst = NULL;
	xmlNodePtr parent;
	xmlNodePtr clone;
	ptrdiff_t i, j;

	position_attr = xmlGetNoNsProp(commandInfo->element, POSITION_ATTR);
	if (position_attr)
	{
		if ((position = appendModeFromString(position_attr)) == POS_ERROR)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid position argument: %s", position_attr), true, true);
			goto done;
		}
	}

	select_attr = xmlGetNoNsProp(commandInfo->element, SELECT_ATTR);
	if (!select_attr)
		select_attr = xmlGetNoNsProp(commandInfo->element, DESTINATION_ATTR);
	if (!select_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "neither select nor destination attribute specified"), true, true);
		goto done;
	}

	source_attr = xmlGetNoNsProp(commandInfo->element, SOURCE_ATTR);
	if (source_attr) /* �� ���� ����� ������... */
	{
		src = xplSelectNodes(commandInfo->document, commandInfo->element, source_attr);
		if (src)
		{
			if (src->type != XPATH_NODESET)
			{
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "source XPath (%s) returned non-nodeset value", source_attr), true, true);
				goto done;
			}
		} else {
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid source XPath (%s)", source_attr), true, true);
			xmlResetLastError();
			goto done;
		}
	} else if (!commandInfo->element->children) {
		/* ������������ ����������� ���, ����� ������ �� ������ */
		ASSIGN_RESULT(NULL, false, true);
		goto done;
	}

	dst = xplSelectNodes(commandInfo->document, commandInfo->element, select_attr);
	if (dst)
	{
		if (dst->type == XPATH_NODESET)
		{
			if (dst->nodesetval)
			{
				for (i = 0; i < (ptrdiff_t) dst->nodesetval->nodeNr; i++)
				{
					if ((position > POS_AFTER) && (dst->nodesetval->nodeTab[i]->type != XML_ELEMENT_NODE))
					/* �� ����� ����������� �������� �������� ���� ������, ����� ������ ������ */
						continue;
					/* ������� ������� ��������� ���� �� � ���� */
					if (dst->nodesetval->nodeTab[i]->type == XML_ATTRIBUTE_NODE)
						continue;
					parent = PARENT(dst->nodesetval->nodeTab[i]);
					if (!src) /* ���������� ���������� ������� */
					{
						clone = cloneNodeList(commandInfo->element->children, parent, commandInfo->element->doc);
						doAppend(clone, dst->nodesetval->nodeTab[i], position);
					} else if (src->nodesetval) {
						/* ���������� �������-�������� */
						if (position == POS_FIRST || position == POS_AFTER)
						{
							/* ��� ����������� ����� �� ������ ������������� ���� ��� ���������� ������� ���������� */
							for (j = (ptrdiff_t) src->nodesetval->nodeNr - 1; j >= 0; j--)
								doAppend(cloneAttrAsText(src->nodesetval->nodeTab[j], parent), dst->nodesetval->nodeTab[i], position);
						} else {
							for (j = 0; j < (ptrdiff_t) src->nodesetval->nodeNr; j++)
								doAppend(cloneAttrAsText(src->nodesetval->nodeTab[j], parent), dst->nodesetval->nodeTab[i], position);
						}
					}
				}
			}
		} else {
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "destination XPath (%s) returned non-nodeset value", select_attr), true, true);
			goto done; 
		}
	} else {
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid destination XPath (%s)", select_attr), true, true);
		xmlResetLastError();
		goto done;
	}

	ASSIGN_RESULT(NULL, false, true);
done:
	if (position_attr) xmlFree(position_attr);
	if (select_attr) xmlFree(select_attr);
	if (source_attr) xmlFree(source_attr);
	if (src) xmlXPathFreeObject(src);
	if (dst) xmlXPathFreeObject(dst);
}

xplCommand xplAppendCommand = { xplCmdAppendPrologue, xplCmdAppendEpilogue };

