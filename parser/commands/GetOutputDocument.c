#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include "commands/GetOutputDocument.h"

void xplCmdGetOutputDocumentPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdGetOutputDocumentEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define SELECT_ATTR (BAD_CAST "select")
#define REPEAT_ATTR (BAD_CAST "repeat")
	xmlChar *select_attr = NULL;
	xmlDocPtr doc = NULL;
	bool repeat;
	xmlXPathObjectPtr sel = NULL;
	size_t i;
	xmlNodePtr sibling, prnt, copy, ret, tail, error;

	if (commandInfo->document->role != XPL_DOC_ROLE_EPILOGUE)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "this command is only available from epilogue wrapper"), true, true);
		goto done;
	}
	if (!commandInfo->document->main || !commandInfo->document->main->document)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "main document wasn't evaluated due to some error"), true, true);
		goto done;
	}
	doc = commandInfo->document->main->document;
	select_attr = xmlGetNoNsProp(commandInfo->element, SELECT_ATTR);
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, true)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}

	if (select_attr)
	{
		sel = xplSelectNodes(commandInfo->document, (xmlNodePtr) doc, select_attr);
		if (sel)
		{
			if (sel->type == XPATH_NODESET)
			{
				if (sel->nodesetval)
				{
					ret = tail = NULL;
					for (i = 0; i < (size_t) sel->nodesetval->nodeNr; i++)
					{
						sibling = sel->nodesetval->nodeTab[i];
						prnt = sibling->parent;
						sibling->parent = NULL;
						copy = xplCloneAsNodeChild(sibling, commandInfo->element);
						sibling->parent = prnt;
						if (!tail)
							tail = copy;
						else
							tail = xmlAddNextSibling(tail, copy);
						if (!ret)
							ret = tail;
					}
				}
			} else if (sel->type != XPATH_UNDEFINED) {
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath expression (%s) evaluated to non-nodeset value", select_attr), true, true);
				goto done;
			} else {
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath expression (%s) evaluated to undef", select_attr), true, true);
				goto done;
			}
		} else {
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid select XPath expression (%s)", select_attr), true, true);
			goto done;
		}
	} else {
		ret = xplCloneNode(doc->children, commandInfo->element, commandInfo->document->document);
		xplDownshiftNodeListNsDef(ret, commandInfo->element->nsDef);
	}
	ASSIGN_RESULT(ret, repeat, true);
done:
	if (select_attr) XPL_FREE(select_attr);
	if (sel) xmlXPathFreeObject(sel);
}

xplCommand xplGetOutputDocumentCommand = { xplCmdGetOutputDocumentPrologue, xplCmdGetOutputDocumentEpilogue };
