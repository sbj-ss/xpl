#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsession.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>
#include "commands/SessionGetObject.h"

void xplCmdSessionGetObjectPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdSessionGetObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define NAME_ATTR (BAD_CAST "name")
#define SELECT_ATTR (BAD_CAST "select")
#define REPEAT_ATTR (BAD_CAST "repeat")
#define THREADLOCAL_ATTR (BAD_CAST "threadlocal")
	xmlChar *name_attr = NULL;
	xmlChar *select_attr = NULL;
	bool repeat;
	bool threadlocal;
	xmlXPathObjectPtr sel = NULL;
	xmlNodePtr obj, head = NULL, tail, cur, error;
	size_t i;

	if (!commandInfo->document->main->session)
	{
		ASSIGN_RESULT(NULL, false, true);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, true)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	name_attr = xmlGetNoNsProp(commandInfo->element, NAME_ATTR);
	if (!name_attr)
	{
		cur = xplSessionGetAllObjects(commandInfo->document->main->session);
		cur = cloneNodeList(cur, commandInfo->element->parent, commandInfo->element->doc);
		ASSIGN_RESULT(cur, repeat, true);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, THREADLOCAL_ATTR, &threadlocal, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if (threadlocal)
		name_attr = appendThreadIdToString(name_attr, xprGetCurrentThreadId());
	select_attr = xmlGetNoNsProp(commandInfo->element, SELECT_ATTR);
	obj = xplSessionGetObject(commandInfo->document->main->session, name_attr);
	if (obj)
	{
		if (select_attr)
		{
			sel = xplSelectNodes(commandInfo->document, obj, select_attr);
			if (sel)
			{
				head = tail = NULL;
				if ((sel->type == XPATH_NODESET) && sel->nodesetval)
				{
					for (i = 0; i < (size_t) sel->nodesetval->nodeNr; i++)
					{
						if (head)
						{
							cur = cloneNode(sel->nodesetval->nodeTab[i], commandInfo->element->parent, commandInfo->element->doc);
							tail->next = cur;
							cur->prev = tail;
							tail = cur;
						} else {
							head = tail = cloneNode(sel->nodesetval->nodeTab[i], commandInfo->element->parent, commandInfo->element->doc);
						}
					}
				} else if (sel->type != XPATH_UNDEFINED) {
					head = xmlNewDocText(commandInfo->element->doc, NULL);
					head->content = xmlXPathCastToString(sel);
					repeat = false;
				} else {
					head = xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath expresssion (%s) evaluated to undef", select_attr);
					repeat = true;
				}
			} else {
				head = xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid select XPath expression (%s)", select_attr);
				repeat = true;
			}
		} else 
			head = cloneNodeList(obj->children, commandInfo->element->parent, commandInfo->element->doc);
	}
	ASSIGN_RESULT(head, repeat, true);
done:
	if (name_attr)
		xmlFree(name_attr);
	if (select_attr)
		xmlFree(select_attr);
	if (sel)
		xmlXPathFreeObject(sel);
}

xplCommand xplSessionGetObjectCommand = { xplCmdSessionGetObjectPrologue, xplCmdSessionGetObjectEpilogue };
