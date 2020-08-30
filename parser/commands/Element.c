#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include "commands/Element.h"

typedef struct _xplCmdElementParams
{
	xplQName name;
	bool repeat;
} xplCmdElementParams, *xplCmdElementParamsPtr;

static const xplCmdElementParams params_stencil =
{
	.name = { NULL, NULL },
	.repeat = true
};

xplCommand xplElementCommand =
{
	.prologue = xplCmdElementPrologue,
	.epilogue = xplCmdElementEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdElementParams),
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.value_stencil = &params_stencil.name
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};

void xplCmdElementPrologue(xplCommandInfoPtr commandInfo)
{
	UNUSED_PARAM(commandInfo);
}

void xplCmdElementEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdElementParamsPtr params = (xplCmdElementParamsPtr) commandInfo->params;
	xmlNodePtr el;

	el = xmlNewDocNode(commandInfo->element->doc, params->name.ns, params->name.ncname, NULL);
	xplSetChildren(el, xplDetachContent(commandInfo->element));
	ASSIGN_RESULT(el, params->repeat, true);
}
