#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsession.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>

void xplCmdSessionGetObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdSessionGetObjectParams
{
	xmlChar *name;
	xmlChar *select;
	bool repeat;
	bool thread_local;
} xplCmdSessionGetObjectParams, *xplCmdSessionGetObjectParamsPtr;

static const xplCmdSessionGetObjectParams params_stencil =
{
	.name = NULL,
	.select = NULL,
	.repeat = true,
	.thread_local = false
};

xplCommand xplSessionGetObjectCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdSessionGetObjectEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdSessionGetObjectParams),
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.name
		}, {
			.name = BAD_CAST "select",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.select
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = BAD_CAST "threadlocal",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.thread_local
		}, {
			.name = NULL
		}
	}
};

void xplCmdSessionGetObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSessionGetObjectParamsPtr params = (xplCmdSessionGetObjectParamsPtr) commandInfo->params;
	xmlXPathObjectPtr sel = NULL;
	xmlNodePtr obj, head, tail, cur;
	size_t i;

	if (!commandInfo->document->main->session)
	{
		ASSIGN_RESULT(NULL, false, true);
		return;
	}
	if (!params->name) // TODO select?..
	{
		obj = xplSessionGetAllObjects(commandInfo->document->main->session);
		obj = xplCloneNodeList(obj, commandInfo->element->parent, commandInfo->element->doc);
		ASSIGN_RESULT(obj, params->repeat, true);
		return;
	}
	if (params->thread_local)
		params->name = xstrAppendThreadIdToString(params->name, xprGetCurrentThreadId());
	obj = xplSessionGetObject(commandInfo->document->main->session, params->name);
	if (obj)
	{
		if (params->select)
		{
			sel = xplSelectNodes(commandInfo, obj, params->select); // note: we can't delegate this to params handling due to the different base
			if (sel)
			{
				head = tail = NULL;
				if ((sel->type == XPATH_NODESET) && sel->nodesetval)
				{
					for (i = 0; i < (size_t) sel->nodesetval->nodeNr; i++)
					{
						cur = xplCloneNode(sel->nodesetval->nodeTab[i], commandInfo->element->parent, commandInfo->element->doc);
						if (head)
						{
							tail->next = cur;
							cur->prev = tail;
							tail = cur;
						} else
							head = tail = cur;
					}
				} else if (sel->type != XPATH_UNDEFINED) {
					head = xmlNewDocText(commandInfo->element->doc, NULL);
					head->content = xmlXPathCastToString(sel);
					params->repeat = false;
				} else {
					head = xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath expresssion (%s) evaluated to undef", params->select);
					params->repeat = true;
				}
				xmlXPathFreeObject(sel);
			} else {
				head = xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid select XPath expression (%s)", params->select);
				params->repeat = true;
			}
		} else 
			head = xplCloneNodeList(obj->children, commandInfo->element->parent, commandInfo->element->doc);
	} else
		head = NULL;
	ASSIGN_RESULT(head, params->repeat && head, true);
}
