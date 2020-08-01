#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include "commands/ForEach.h"

/* A short note for those wanting to replace children with error node inside prologue:
 * 1. the error node must replace the command and not reside inside it
 * 2. doube error processing is an error, too
 */
void xplCmdForEachPrologue(xplCommandInfoPtr commandInfo)
{
#define SELECT_ATTR (BAD_CAST "select")
#define ID_ATTR (BAD_CAST "id")
	xmlChar *select_attr = NULL;
	xmlChar *id_attr = NULL;
	xmlXPathObjectPtr sel = NULL;
	size_t i;
	xmlNodePtr cur, tail, repl, ret = NULL;
	
	select_attr = xmlGetNoNsProp(commandInfo->element, SELECT_ATTR);
	if (!select_attr)
	{
		ret = NULL;
		commandInfo->prologue_error = xplCreateErrorNode(commandInfo->element, BAD_CAST "missing select attribute");
		goto done;
	}
	id_attr = xmlGetNoNsProp(commandInfo->element, ID_ATTR);

	sel = xplSelectNodes(commandInfo, commandInfo->element, select_attr);
	if (sel)
	{
		if (sel->type == XPATH_NODESET)
		{
			if (sel->nodesetval)
			{
				for (i = 0; i < (size_t) sel->nodesetval->nodeNr; i++)
				{
					cur = sel->nodesetval->nodeTab[i];
					if ((cur->type != XML_ELEMENT_NODE) && (cur->type != XML_ATTRIBUTE_NODE))
						continue;
					repl = xplReplaceContentEntries(commandInfo->document, id_attr, cur, commandInfo->element->children); 
					if (!ret)
						ret = repl;
					else {
						tail->next = repl;
						repl->prev = tail;
					}
					tail = xplFindTail(repl);
				}
			}
		} else {
			ret = NULL;
			commandInfo->prologue_error = xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath (%s) evaluated to non-nodeset value", select_attr);
			goto done;
		}
	} else {
		ret = NULL;
		commandInfo->prologue_error = xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid select XPath expression (%s)", select_attr);
		goto done;
	}
done:
	xplDocDeferNodeListDeletion(commandInfo->document, xplDetachContent(commandInfo->element));
	xplSetChildren(commandInfo->element, ret);	
	if (select_attr) XPL_FREE(select_attr);
	if (id_attr) xmlFree (id_attr);
	if (sel) xmlXPathFreeObject(sel);
}

void xplCmdForEachEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define REPEAT_ATTR (BAD_CAST "repeat")
	bool repeat;
	xmlNodePtr error;

	if (commandInfo->prologue_error)
	{
		ASSIGN_RESULT(commandInfo->prologue_error, true, true);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, false)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	ASSIGN_RESULT(xplDetachContent(commandInfo->element), repeat, true);
}

xplCommand xplForEachCommand = { 
	.prologue = xplCmdForEachPrologue, 
	.epilogue = xplCmdForEachEpilogue,
	.initializer = NULL,
	.finalizer = NULL,
	.flags = 0 /* we don't need "content safe flag here */
};
