#include <libxpl/xplcore.h>
#include <libxpl/xplmacro.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdReplaceIfUndefinedPrologue(xplCommandInfoPtr commandInfo);
void xplCmdReplaceIfUndefinedEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdReplaceIfUndefinedParams
{
	xplQName name;
} xplCmdReplaceIfUndefinedParams, *xplCmdReplaceIfUndefinedParamsPtr;

static const xplCmdReplaceIfUndefinedParams params_stencil =
{
	.name = { NULL, NULL }
};

xplCommand xplReplaceIfUndefinedCommand = {
	.prologue = xplCmdReplaceIfUndefinedPrologue,
	.epilogue = xplCmdReplaceIfUndefinedEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_PROLOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdReplaceIfUndefinedParams),
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.required = true,
			.value_stencil = &params_stencil.name
		}, {
			.name = NULL
		}
	}
};

void xplCmdReplaceIfUndefinedPrologue(xplCommandInfoPtr commandInfo)
{
	xplCmdReplaceIfUndefinedParamsPtr params = (xplCmdReplaceIfUndefinedParamsPtr) commandInfo->params;

	if (xplMacroLookupByQName(commandInfo->element->parent, params->name))
	{
		xplDocDeferNodeListDeletion(commandInfo->document, xplDetachChildren(commandInfo->element));
		/* use prologue_error as a buffer for the macro trigger node */
		commandInfo->prologue_error = xmlNewDocNode(commandInfo->element->doc, params->name.ns, params->name.ncname, NULL);
	}
}

void xplCmdReplaceIfUndefinedEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	if (commandInfo->prologue_error)
		ASSIGN_RESULT(commandInfo->prologue_error, true, true);
	else
		ASSIGN_RESULT(xplDetachChildren(commandInfo->element), false, true);
}
