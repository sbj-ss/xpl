#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdReplicatePrologue(xplCommandInfoPtr commandInfo);
void xplCmdReplicateEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdReplicateParams
{
	int before_count;
	int after_count;
	bool repeat;
} xplCmdReplicateParams, *xplCmdReplicateParamsPtr;

static const xplCmdReplicateParams params_stencil =
{
	.before_count = 1,
	.after_count = 1,
	.repeat = false
};

xplCommand xplReplicateCommand =
{
	.prologue = xplCmdReplicatePrologue,
	.epilogue = xplCmdReplicateEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_PROLOGUE | XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdReplicateParams),
	.parameters = {
		{
			.name = BAD_CAST "beforecount",
			.type = XPL_CMD_PARAM_TYPE_INT,
			.value_stencil = &params_stencil.before_count
		}, {
			.name = BAD_CAST "aftercount",
			.type = XPL_CMD_PARAM_TYPE_INT,
			.value_stencil = &params_stencil.after_count
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};

static xmlNodePtr _replicateNodes(xmlNodePtr src, int count, xmlNodePtr parent)
{
	xmlNodePtr cur, tail = NULL, ret = NULL;
	int i;

	if (!src)
		return NULL;
	for (i = 0; i < count; i++)
	{
		cur = xplCloneNodeList(src, parent, src->doc);
		APPEND_NODE_TO_LIST(ret, tail, cur);
		tail = xplFindTail(tail);
	}
	return ret;
}

void xplCmdReplicatePrologue(xplCommandInfoPtr commandInfo)
{
	xplCmdReplicateParamsPtr params = (xplCmdReplicateParamsPtr) commandInfo->params;
	xmlNodePtr old_children, new_children = NULL;

	if (params->before_count != 1)
	{
		old_children = xplDetachChildren(commandInfo->element);
		if (params->before_count > 1)
			new_children = _replicateNodes(old_children, params->before_count, commandInfo->element);
		xplDocDeferNodeListDeletion(commandInfo->document, old_children);
		xplSetChildren(commandInfo->element, new_children);
	}
}

void xplCmdReplicateEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdReplicateParamsPtr params = (xplCmdReplicateParamsPtr) commandInfo->params;
	xmlNodePtr ret = NULL;

	if (params->after_count > 1)
		ret = _replicateNodes(commandInfo->element->children, params->after_count, commandInfo->element->parent);
	else if (params->after_count == 1)
		ret = xplDetachChildren(commandInfo->element);
	else
		ret = NULL;
	ASSIGN_RESULT(ret, params->repeat, true);
}
