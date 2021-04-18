#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdForEachPrologue(xplCommandInfoPtr commandInfo);
void xplCmdForEachEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdForEachParams
{
	xmlXPathObjectPtr select;
	xmlChar *id;
	bool repeat;
} xplCmdForEachParams, *xplCmdForEachParamsPtr;

static const xplCmdForEachParams params_stencil =
{
	.select = NULL,
	.id = NULL,
	.repeat = false
};

xplCommand xplForEachCommand = {
	.prologue = xplCmdForEachPrologue,
	.epilogue = xplCmdForEachEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_PROLOGUE | XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_HAS_CONTENT,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdForEachParams),
	.parameters = {
		{
			.name = BAD_CAST "select",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_NODESET,
			.required = true,
			.value_stencil = &params_stencil.select
		}, {
			.name = CONTENT_ID_ATTR,
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.id
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};

void xplCmdForEachPrologue(xplCommandInfoPtr commandInfo)
{
	xplCmdForEachParamsPtr params = (xplCmdForEachParamsPtr) commandInfo->params;
	size_t i;
	xmlNodePtr cur, tail = NULL, repl, ret = NULL;
	
	if (params->select->nodesetval)
	{
		for (i = 0; i < (size_t) params->select->nodesetval->nodeNr; i++)
		{
			cur = params->select->nodesetval->nodeTab[i];
			if ((cur->type != XML_ELEMENT_NODE) && (cur->type != XML_ATTRIBUTE_NODE))
			{
				if (cfgWarnOnInvalidNodeType)
					xplDisplayWarning(commandInfo->element, "can only process elements and attributes, select '%s'", (char*) params->select->user);
				continue;
			}
			repl = xplReplaceContentEntries(commandInfo->document, params->id, cur, commandInfo->element->children, commandInfo->element);
			if (!ret)
				ret = repl;
			else {
				tail->next = repl;
				repl->prev = tail;
			}
			tail = xplFindTail(repl);
		}
	}
	xplDocDeferNodeListDeletion(commandInfo->document, xplDetachChildren(commandInfo->element));
	xplSetChildren(commandInfo->element, ret);	
}

void xplCmdForEachEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdForEachParamsPtr params = (xplCmdForEachParamsPtr) commandInfo->params;

	ASSIGN_RESULT(xplDetachChildren(commandInfo->element), params->repeat, true);
}
