#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdDeleteEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdDeleteParams
{
	xmlXPathObjectPtr select;
} xplCmdDeleteParams, *xplCmdDeleteParamsPtr;

static const xplCmdDeleteParams params_stencil =
{
	.select = NULL
};

xplCommand xplDeleteCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdDeleteEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdDeleteParams),
	.parameters = {
		{
			.name = BAD_CAST "select",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.required = true,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_NODESET,
			.value_stencil = &params_stencil.select
		}, {
			.name = NULL
		}
	}
};

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
	xplCmdDeleteParamsPtr params = (xplCmdDeleteParamsPtr) commandInfo->params;
	xmlNodeSetPtr nodes;
	xmlNodePtr cur;
	size_t i, j;
	bool double_pass_mode;
	rbBufPtr deleted_buf = NULL;
	
	if ((nodes = params->select->nodesetval))
	{
		double_pass_mode = cfgFoolproofDestructiveCommands
			&& (nodes->nodeNr > MAX_ELEMENTS_FOR_RELATIONSHIP_SCAN)
			&& !commandInfo->document->iterator_spinlock;
		if (!double_pass_mode)
		{
			if (cfgFoolproofDestructiveCommands)
			{
				for (i = 0; i < (size_t) nodes->nodeNr; i++)
					for (j = i + 1; j < (size_t) nodes->nodeNr; j++)
						if (xplIsAncestor(nodes->nodeTab[j], nodes->nodeTab[i]))
							nodes->nodeTab[j] = NULL;
			}
		} else
			deleted_buf = rbCreateBufParams((size_t) sizeof(xmlNodePtr)*params->select->nodesetval->nodeNr, RB_GROW_FIXED, 0);
		for (i = 0; i < (size_t) nodes->nodeNr; i++)
		{
			cur = nodes->nodeTab[i];
			if (!cur)
				continue; /* already removed with its parent */
			if ((int) cur->type & XPL_NODE_DELETION_MASK)
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
					if (xplIsAncestor(commandInfo->element, cur) || (commandInfo->element == cur)) /* self/parent deletion request */
					{
						if (commandInfo->document->iterator_spinlock) /* mark for deletion at the end of processing */
							xplDocDeferNodeDeletion(commandInfo->document, cur);
						xplMarkAncestorAxisForDeletion(commandInfo->element, cur);
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
					/* attributes can't have child nodes */
					xplUnlinkProp((xmlAttrPtr) cur);
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
	ASSIGN_RESULT(NULL, false, true);
}
