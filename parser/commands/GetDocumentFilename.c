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

static xmlChar* _getDocument(xplCommandInfoPtr info, const xmlChar *raw_value, void **result)
{
	if (!xplGetDocByRole(info->document, raw_value, (xplDocumentPtr*) result))
		return xplFormatMessage(BAD_CAST "invalid document attribute value '%s'", raw_value);
	return NULL;
}

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
					.getter = _getDocument,
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
