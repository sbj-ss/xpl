#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdReplicatePrologue(xplCommandInfoPtr commandInfo);
void xplCmdReplicateEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

static xmlNodePtr _replicateNodes(xmlNodePtr src, int count, xmlNodePtr parent)
{
	xmlNodePtr cur, tail, ret = NULL;
	int i;

	if (!src)
		return NULL;
	for (i = 0; i < count; i++)
	{
		cur = xplCloneNodeList(src, parent, src->doc);
#ifdef _DEBUG
		if (!cur)
		{
			DISPLAY_INTERNAL_ERROR_MESSAGE();
			xprDebugBreak();
		}
#endif
		if (!ret) 
		{
			ret = tail = cur;
		}
		else {
			tail->next = cur;
			cur->prev = tail;
		}
		tail = xplFindTail(tail);
	}
	return ret;
}

void xplCmdReplicatePrologue(xplCommandInfoPtr commandInfo)
{
#define BEFORE_COUNT_ATTR (BAD_CAST "beforecount")

	xmlChar *before_count_attr = NULL;
	int before_count = 1;
	xmlNodePtr old_children, new_children = NULL;

	before_count_attr = xmlGetNoNsProp(commandInfo->element, BEFORE_COUNT_ATTR);
	if (before_count_attr)
	{
		if (sscanf((const char*) before_count_attr, "%d", &before_count) != 1)
		{
			commandInfo->prologue_error = xplCreateErrorNode(commandInfo->element, BAD_CAST "beforecount (%s) is not a number", before_count_attr);
			xplDocDeferNodeListDeletion(commandInfo->document, xplDetachContent(commandInfo->element));
			goto done;
		}
	}
	if (before_count != 1)
	{
		old_children = xplDetachContent(commandInfo->element);
		if (before_count > 1)
			new_children = _replicateNodes(old_children, before_count, commandInfo->element);
		xplDocDeferNodeListDeletion(commandInfo->document, old_children);
		xplSetChildren(commandInfo->element, new_children);
	}
done:
	if (before_count_attr) XPL_FREE(before_count_attr);
}

void xplCmdReplicateEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define AFTER_COUNT_ATTR (BAD_CAST "aftercount")
#define REPEAT_ATTR (BAD_CAST "repeat")

	xmlChar *after_count_attr = NULL;
	int after_count = 1;
	bool repeat;
	xmlNodePtr ret = NULL, error;

	if (commandInfo->prologue_error)
	{
		ASSIGN_RESULT(commandInfo->prologue_error, true, true);
		goto done;
	}

	after_count_attr = xmlGetNoNsProp(commandInfo->element, AFTER_COUNT_ATTR);
	if (after_count_attr)
	{
		if (sscanf((const char*) after_count_attr, "%d", &after_count) != 1)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "aftercount (%s) is not a number", after_count_attr), true, true);
			goto done;
		}
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if (after_count > 1)
		ret = _replicateNodes(commandInfo->element->children, after_count, commandInfo->element->parent);
	else if (after_count == 1) {
		ret = xplDetachContent(commandInfo->element);
		xplDownshiftNodeListNsDef(ret, commandInfo->element->nsDef);
	} else
		ret = NULL;
	ASSIGN_RESULT(ret, repeat, true);	
done:
	if (after_count_attr)
		XPL_FREE(after_count_attr);
}

xplCommand xplReplicateCommand = { xplCmdReplicatePrologue, xplCmdReplicateEpilogue };
