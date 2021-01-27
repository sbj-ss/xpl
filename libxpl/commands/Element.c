#include <libxpl/xplcommand.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdElementEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

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
	.prologue = NULL,
	.epilogue = xplCmdElementEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdElementParams),
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.required = true,
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

void xplCmdElementEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdElementParamsPtr params = (xplCmdElementParamsPtr) commandInfo->params;
	xmlNsPtr ns;
	bool own_ns = false;
	xmlNodePtr el;

	if (params->name.ns)
	{
		 ns = commandInfo->element->nsDef;
		 while (ns)
		 {
			 if (ns == params->name.ns)
			 {
				 params->name.ns = xmlCopyNamespace(ns);
				 own_ns = true;
				 break;
			 }
			 ns = ns->next;
		 }
	}
	el = xmlNewDocNode(commandInfo->element->doc, params->name.ns, params->name.ncname, NULL);
	if (own_ns)
		el->nsDef = params->name.ns;
	xplSetChildren(el, xplDetachContent(commandInfo->element));
	ASSIGN_RESULT(el, params->repeat, true);
}
