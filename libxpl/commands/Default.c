#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>

void xplCmdDefaultPrologue(xplCommandInfoPtr commandInfo);
void xplCmdDefaultEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdDefaultParams
{
	bool repeat;
	bool do_break;
} xplCmdDefaultParams, *xplCmdDefaultParamsPtr;

static const xplCmdDefaultParams params_stencil =
{
	.repeat = false,
	.do_break = true
};

xplCommand xplDefaultCommand =
{
	.prologue = xplCmdDefaultPrologue,
	.epilogue = xplCmdDefaultEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdDefaultParams),
	.parameters = {
		{
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = BAD_CAST "break",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.do_break
		}, {
			.name = NULL
		}
	}
};

void xplCmdDefaultPrologue(xplCommandInfoPtr commandInfo)
{
	xmlNodePtr parent = commandInfo->element->parent;

	if (!xplCheckNodeForXplNs(commandInfo->document, parent) || xmlStrcmp(parent->name, BAD_CAST "switch"))
	{
		commandInfo->prologue_error = xplCreateErrorNode(commandInfo->element, "parent tag must be :switch");
		xplDocDeferNodeListDeletion(commandInfo->document, xplDetachChildren(commandInfo->element));
	}
}

void xplCmdDefaultEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdDefaultParamsPtr params = (xplCmdDefaultParamsPtr) commandInfo->params;

	if (commandInfo->prologue_error)
	{
		ASSIGN_RESULT(commandInfo->prologue_error, true, true);
		return;
	}
	ASSIGN_RESULT(xplDetachChildren(commandInfo->element), params->repeat, true);
	if (params->do_break)
	{
		xplDocDeferNodeListDeletion(commandInfo->document, commandInfo->element->next);
		commandInfo->element->next = NULL;
		commandInfo->element->parent->last = commandInfo->element;
	}
}
