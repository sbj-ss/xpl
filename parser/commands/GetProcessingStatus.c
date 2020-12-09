#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include "commands/GetProcessingStatus.h"

typedef struct _xplCmdGetProcessingStatusParams
{
	xplDocumentPtr document;
} xplCmdGetProcessingStatusParams, *xplCmdGetProcessingStatusParamsPtr;

static const xplCmdGetProcessingStatusParams params_stencil =
{
	.document = NULL
};

xplCommand xplGetProcessingStatusCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdGetProcessingStatusEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdGetProcessingStatusParams),
	.parameters = {
		{
			.name = BAD_CAST "document",
			.type = XPL_CMD_PARAM_TYPE_PTR_CUSTOM_GETTER,
			.extra.ptr_fn.getter = xplDocByRoleGetter,
			.value_stencil = &params_stencil.document
		}, {
			.name = NULL
		}
	}
};

void xplCmdGetProcessingStatusEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdGetProcessingStatusParamsPtr params = (xplCmdGetProcessingStatusParamsPtr) commandInfo->params;
	xmlNodePtr ret;
	const xmlChar *status;

	status = params->document? xplErrorToShortString(params->document->status): BAD_CAST "no_such_doc";
	ret = xmlNewDocText(commandInfo->element->doc, status);
	ASSIGN_RESULT(ret, false, true);
}
