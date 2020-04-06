#include "commands/Replicate.h"
#include "Core.h"
#include "Messages.h"
#include "Utils.h"

xmlNodePtr replicateNodes(xmlNodePtr src, int count, xmlNodePtr parent)
{
	xmlNodePtr cur, tail, ret = NULL;
	int i;

	if (!src)
		return NULL;
	for (i = 0; i < count; i++)
	{
		cur = cloneNodeList(src, parent, src->doc);
#ifdef _DEBUG
		if (!cur)
		{
			DISPLAY_INTERNAL_ERROR_MESSAGE
			__debugbreak();
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
		tail = findTail(tail);
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
			commandInfo->_private = xplCreateErrorNode(commandInfo->element, BAD_CAST "beforecount (%s) is not a number", before_count_attr);
			xplDocDeferNodeListDeletion(commandInfo->document, detachContent(commandInfo->element));
			goto done;
		}
	}
	if (before_count != 1)
	{
		old_children = detachContent(commandInfo->element);
		if (before_count > 1)
			new_children = replicateNodes(old_children, before_count, commandInfo->element);
		xplDocDeferNodeListDeletion(commandInfo->document, old_children);
		setChildren(commandInfo->element, new_children);
	}
done:
	if (before_count_attr) xmlFree(before_count_attr);
}

void xplCmdReplicateEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define AFTER_COUNT_ATTR (BAD_CAST "aftercount")
#define REPEAT_ATTR (BAD_CAST "repeat")

	xmlChar *after_count_attr = NULL;
	int after_count = 1;
	bool repeat;
	xmlNodePtr ret = NULL, error;

	if (commandInfo->_private)
	{
		ASSIGN_RESULT(commandInfo->_private, true, true);
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
		ret = replicateNodes(commandInfo->element->children, after_count, commandInfo->element->parent);
	else if (after_count == 1) {
		ret = detachContent(commandInfo->element);
		downshiftNodeListNsDef(ret, commandInfo->element->nsDef);
	} else
		ret = NULL;
	ASSIGN_RESULT(ret, repeat, true);	
done:
	if (after_count_attr)
		xmlFree(after_count_attr);
}

xplCommand xplReplicateCommand = { xplCmdReplicatePrologue, xplCmdReplicateEpilogue };
