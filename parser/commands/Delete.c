#include "commands/Delete.h"
#include "Core.h"
#include "Messages.h"
#include "Options.h"
#include "Utils.h"

#if 0

inline void printIndent(int indent)
{
	int i = indent;
	while(i--)
		printf(" ");
}

void dumpNode(xmlNodePtr cur, int indent);
void dumpNodeList(xmlNodePtr cur, int indent)
{
	while (cur)
	{
		dumpNode(cur, indent);
		cur = cur->next;
	}
}

void dumpNode(xmlNodePtr cur, int indent)
{
	printIndent(indent);
	if (cur->type == XML_ELEMENT_NODE && cur->properties)
	{
		printf("element \"%s\" (%08X) ns %08X nsdef %08X content %08X\n", cur->name, cur->name, cur->ns, cur->nsDef, cur->content);
		dumpNodeList((xmlNodePtr) cur->properties, indent + 2);
		dumpNodeList(cur->children, indent + 2);
	} else if (cur->type == XML_TEXT_NODE) {
		printf("text \"%s\" (%08X) value \"%s\" (%08X)\n", cur->name, cur->name, cur->content, cur->content);
	} else if (cur->type == XML_COMMENT_NODE) {
		printf("comment \"%s\" (%08X) value \"%s\" (%08X)\n", cur->name, cur->name, cur->content, cur->content);
	} else if (cur->type == XML_ATTRIBUTE_NODE) {
		xmlAttrPtr prop = (xmlAttrPtr) cur;
		printf("attribute %s (%08X) ns %08X atype %d psvi %08X\n", prop->name, prop->name, prop->ns, prop->atype, prop->psvi);
		dumpNodeList(cur->children, indent + 2);
	} else {
		printIndent(indent);
		printf("!!!!!!!! node type: %d\n", cur->type);
	}
}
#endif

void xplCmdDeletePrologue(xplCommandInfoPtr commandInfo)
{
}

/* Необходимо наконец навести порядок с пилением сука, на котором сидит команда.
 * Возможные гадости включают в себя:
 * а) удаление элемента, являющегося предком следующего узла выборки;
 * б) удаление себя/предка;
 * в) удаление узла, входящего в выборку выполняющегося итератора.
 * Выделим две ситуации:
 * 1. Выполнение из итератора (в этом случае узлы не удаляются вообще, а помечаются к удалению
 *    в конце обработки документа через его буфер);
 * 2. Выполнение в общем (противном) случае
 *   2.1. При количестве узлов <= MAX_ELEMENTS_FOR_RELATIONSHIP_SCAN вычисляем треугольник от
 *        декартова произведения узлов. Если узел является предком - удаляем потомков из выборки;
 *   2.2. Иначе используем локальный буфер удаления и осуществляем два прохода (отметка и удаление).
 * Проверка на удаление родителя остаётся и при попытке так сделать маркирует под удаление
 * фрагмент иерархической оси от команды до родителя вне зависимости от выбора в пп. 1-2.
 * 
 * --ss, 28.12.2013 
 */

#define MAX_ELEMENTS_FOR_RELATIONSHIP_SCAN 8

void xplCmdDeleteEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define SELECT_ATTR (BAD_CAST "select")
	xmlChar *select_attr = NULL;
	xmlXPathObjectPtr dest_list = NULL;
	size_t i, j;
	bool double_pass_mode;
	ReszBufPtr deleted_buf = NULL;
	
	select_attr = xmlGetNoNsProp(commandInfo->element, SELECT_ATTR);
	if (!select_attr || !*select_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select attribute is missing or empty"), true, true);
		goto done;
	}
	dest_list = xplSelectNodes(commandInfo->document, commandInfo->element, select_attr);
	if (dest_list)
	{
		if (dest_list->type == XPATH_NODESET)
		{
			if (dest_list->nodesetval)
			{
				double_pass_mode = cfgFoolproofDestructiveCommands
					&& (dest_list->nodesetval->nodeNr > MAX_ELEMENTS_FOR_RELATIONSHIP_SCAN)
					&& !commandInfo->document->iterator_spinlock;
				if (!double_pass_mode)
				{
					if (cfgFoolproofDestructiveCommands)
					{
						for (i = 0; i < (size_t) dest_list->nodesetval->nodeNr; i++)
							for (j = i + 1; j < (size_t) dest_list->nodesetval->nodeNr; j++)
								if (isAncestor(dest_list->nodesetval->nodeTab[j], dest_list->nodesetval->nodeTab[i]))
									dest_list->nodesetval->nodeTab[j] = 0;
					}
				} else
					deleted_buf = rbCreateBufParams((size_t) sizeof(xmlNodePtr)*dest_list->nodesetval->nodeNr, RESZ_BUF_GROW_FIXED, 0);
				for (i = 0; i < (size_t) dest_list->nodesetval->nodeNr; i++)
				{
					xmlNodePtr cur = dest_list->nodesetval->nodeTab[i];
					if (!cur)
						continue; /* already removed with its parent */
					if ((int) cur->type & XML_NODE_DELETION_MASK)
					{
						if (cfgWarnOnDeletedNodeReference)
							xplDisplayMessage(xplMsgWarning, 
								BAD_CAST "node \"%s\" (line %d) post-mortem access attempt (file \"%s\", line %d)", 
								cur->name, 
								cur->line,
								commandInfo->document->document->URL, 
								commandInfo->element->line);
						continue;
					}
					switch (cur->type)
					{
						case XML_ELEMENT_NODE:
							if (isAncestor(commandInfo->element, cur) || (commandInfo->element == cur)) /* self/parent deletion request */
							{
								if (commandInfo->document->iterator_spinlock) /* mark for deletion at the end of processing */
									xplDocDeferNodeDeletion(commandInfo->document, cur);								
								markAncestorAxisForDeletion(commandInfo->element, cur);								
							} else {
								if (double_pass_mode)
									xplDeferNodeDeletion(deleted_buf, cur);
								else
									xplDocDeferNodeDeletion(commandInfo->document, cur);								
							}
							break;
						case XML_TEXT_NODE:
						case XML_CDATA_SECTION_NODE:
						case XML_COMMENT_NODE:
						case XML_PI_NODE:
							if (double_pass_mode)
								xplDeferNodeDeletion(deleted_buf, cur);
							else
								xplDocDeferNodeDeletion(commandInfo->document, cur);
							break;
						case XML_ATTRIBUTE_NODE:
							/* attribures can't have child nodes */
							unlinkProp((xmlAttrPtr) cur);
							xplDocDeferNodeDeletion(commandInfo->document, cur);
							break;
						default:
							/* ToDo: what else?.. we can't touch DTDs anyway */
							break;						
					} /* switch */
				} /* for: first pass */
				if (double_pass_mode)
				{
					xplDeleteDeferredNodes(deleted_buf);
					rbFreeBuf(deleted_buf);
				}
			} /* if nodesetval is present */
		} else {
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath (%s) evaluated to scalar or undefined", select_attr), true, true);
			goto done;
		}
	} else {
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid select XPath expression (%s)", select_attr), true, true);
		goto done;
	}
	ASSIGN_RESULT(NULL, false, true);
done:
	if (select_attr) xmlFree(select_attr);
	if (dest_list) 
	{
		if (dest_list->nodesetval)
			dest_list->nodesetval->nodeNr = 0;
		xmlXPathFreeObject(dest_list);
	}
}

xplCommand xplDeleteCommand = { xplCmdDeletePrologue, xplCmdDeleteEpilogue };
