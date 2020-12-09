#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include "commands/GetOutputDocument.h"

typedef struct _xplCmdGetOutputDocumentParams
{
	xmlChar *select; /* we can't use XPL_CMD_PARAM_TYPE_XPATH here as we'll be selecting from a different document */
	bool repeat;
} xplCmdGetOutputDocumentParams, *xplCmdGetOutputDocumentParamsPtr;

static const xplCmdGetOutputDocumentParams params_stencil =
{
	.select = NULL,
	.repeat = true
};

xplCommand xplGetOutputDocumentCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdGetOutputDocumentEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdGetOutputDocumentParams),
	.parameters = {
		{
			.name = BAD_CAST "select",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.select
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};

void xplCmdGetOutputDocumentEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdGetOutputDocumentParamsPtr params = (xplCmdGetOutputDocumentParamsPtr) commandInfo->params;
	xmlDocPtr doc = NULL;
	xmlXPathObjectPtr sel = NULL;
	size_t i;
	xmlNodePtr sibling, prnt, copy, ret, tail;

	if (commandInfo->document->role != XPL_DOC_ROLE_EPILOGUE)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "this command is only available from epilogue wrapper"), true, true);
		return;
	}
	if (!commandInfo->document->main || !commandInfo->document->main->document)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "main document wasn't evaluated due to some error"), true, true);
		return;
	}
	doc = commandInfo->document->main->document;

	if (params->select)
	{
		sel = xplSelectNodes(commandInfo, doc->children, params->select);
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
				} else
					ret = NULL;
			} else if (sel->type != XPATH_UNDEFINED) { // TODO aren't scalar values OK here?
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath expression (%s) evaluated to non-nodeset value", params->select), true, true);
				goto done;
			} else {
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "select XPath expression (%s) evaluated to undefined", params->select), true, true);
				goto done;
			}
		} else {
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid select XPath expression (%s)", params->select), true, true);
			goto done;
		}
	} else
		ret = xplCloneNode(doc->children, commandInfo->element, commandInfo->document->document);
	ASSIGN_RESULT(ret, params->repeat, true);
done:
	if (sel)
		xmlXPathFreeObject(sel);
}
