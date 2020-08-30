#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>
#include "commands/Case.h"

typedef enum _xplCmdCaseComparison
{
	XPL_CMD_CASE_COMP_EQUALITY,
	XPL_CMD_CASE_COMP_IDENTITY
} xplCmdCaseComparison;

typedef struct _xplCmdCaseParams
{
	xmlXPathObjectPtr key;
	bool do_break;
	bool repeat;
	xplCmdCaseComparison comparison;
} xplCmdCaseParams, *xplCmdCaseParamsPtr;

static const xplCmdCaseParams params_stencil =
{
	.key = NULL,
	.do_break = true,
	.repeat = false,
	.comparison = XPL_CMD_CASE_COMP_EQUALITY
};

static xplCmdParamDictValue comparison_dict[] =
{
	{ BAD_CAST "equality", XPL_CMD_CASE_COMP_EQUALITY },
	{ BAD_CAST "identity", XPL_CMD_CASE_COMP_IDENTITY }
};

xplCommand xplCaseCommand =
{
	.prologue = xplCmdCasePrologue,
	.epilogue = xplCmdCaseEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_PROLOGUE | XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdCaseParams),
	.parameters = {
		{
			.name = BAD_CAST "key",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.required = true,
			.extra = {
				.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_ANY
			},
			.value_stencil = &params_stencil.key
		}, {
			.name = BAD_CAST "break",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.do_break
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = BAD_CAST "comparison",
			.type = XPL_CMD_PARAM_TYPE_DICT,
			.extra = {
				.dict_values = comparison_dict
			},
			.value_stencil = &params_stencil.comparison
		}, {
			.name = NULL
		}
	}
};

void xplCmdCasePrologue(xplCommandInfoPtr commandInfo)
{
	xplCmdCaseParamsPtr params = (xplCmdCaseParamsPtr) commandInfo->params;
	xmlXPathObjectPtr parent_sel;
	xmlNodePtr parent = commandInfo->element->parent;

	if (!xplCheckNodeForXplNs(commandInfo->document, parent) ||	xmlStrcmp(parent->name, BAD_CAST "switch"))
	{
		commandInfo->prologue_error = xplCreateErrorNode(commandInfo->element, BAD_CAST "parent element must be a switch command");
		goto done;
	}
	parent_sel = (xmlXPathObjectPtr) parent->content;
	if (xplCompareXPathSelections(params->key, parent_sel, params->comparison == XPL_CMD_CASE_COMP_EQUALITY))
	{
		if (params->do_break)
		{
			xplDocDeferNodeListDeletion(commandInfo->document, commandInfo->element->next);
			commandInfo->element->next = NULL;
			parent->last = commandInfo->element;
		}
	} else
		xplDocDeferNodeListDeletion(commandInfo->document, xplDetachContent(commandInfo->element));
done:
	if (commandInfo->prologue_error)
		xplDocDeferNodeListDeletion(commandInfo->document, xplDetachContent(commandInfo->element));
}

void xplCmdCaseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdCaseParamsPtr params = (xplCmdCaseParamsPtr) commandInfo->params;

	if (commandInfo->prologue_error)
		ASSIGN_RESULT(commandInfo->prologue_error, true, true);
	else
		ASSIGN_RESULT(xplDetachContent(commandInfo->element), params->repeat, true);
}
