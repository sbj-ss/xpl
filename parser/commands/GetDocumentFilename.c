#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include "commands/GetDocumentFilename.h"

typedef struct _xplCmdGetDocumentFilenameParams
{
	xplDocumentPtr document;
} xplCmdGetDocumentFilenameParams, *xplCmdGetDocumentFilenameParamsPtr;

static const xplCmdGetDocumentFilenameParams params_stencil =
{
	.document = NULL
};

xplCommand xplGetDocumentFilenameCommand =
{
	.prologue = xplCmdGetDocumentFilenamePrologue,
	.epilogue = xplCmdGetDocumentFilenameEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdGetDocumentFilenameParams),
	.parameters = {
		{
			.name = BAD_CAST "document",
			.type = XPL_CMD_PARAM_TYPE_PTR_CUSTOM_GETTER,
			.extra = {
				.ptr_fn = {
					.getter = xplDocByRoleGetter,
					.deallocator = NULL
				}
			},
			.value_stencil = &params_stencil.document
		}, {
			.name = NULL
		}
	}
};

void xplCmdGetDocumentFilenamePrologue(xplCommandInfoPtr commandInfo)
{
	UNUSED_PARAM(commandInfo);
}

void xplCmdGetDocumentFilenameEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdGetDocumentFilenameParamsPtr params = (xplCmdGetDocumentFilenameParamsPtr) commandInfo->params;
	xmlNodePtr ret;
	xmlChar *fn;

	if (!params->document->filename)
		fn = BAD_CAST "<unknown>";
	else
		fn = params->document->filename + xmlStrlen(xplGetDocRoot());
	ret = xmlNewDocText(commandInfo->element->doc, fn);
	ASSIGN_RESULT(ret, false, true);
}
