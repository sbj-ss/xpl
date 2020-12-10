#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>

void xplCmdGetDocumentSourceEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdGetDocumentSourceParams
{
	xplDocumentPtr document;
} xplCmdGetDocumentSourceParams, *xplCmdGetDocumentSourceParamsPtr;

static const xplCmdGetDocumentSourceParams params_stencil =
{
	.document = NULL
};

xplCommand xplGetDocumentSourceCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdGetDocumentSourceEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdGetDocumentSourceParams),
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

void xplCmdGetDocumentSourceEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdGetDocumentSourceParamsPtr params = (xplCmdGetDocumentSourceParamsPtr) commandInfo->params;
	xplDocSource src;
	xmlNodePtr ret;

	src = params->document? params->document->source: XPL_DOC_SOURCE_ABSENT;
	ret = xmlNewDocText(commandInfo->element->doc, xplDocSourceToString(src));
	ASSIGN_RESULT(ret, false, true);
}
