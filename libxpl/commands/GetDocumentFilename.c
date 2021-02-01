#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>

void xplCmdGetDocumentFilenameEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdGetDocumentFilenameParams
{
	bool abs_path;
	xplDocumentPtr document;
} xplCmdGetDocumentFilenameParams, *xplCmdGetDocumentFilenameParamsPtr;

static const xplCmdGetDocumentFilenameParams params_stencil =
{
	.abs_path = false,
	.document = NULL
};

xplCommand xplGetDocumentFilenameCommand =
{
	.prologue = NULL,
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
			.name = BAD_CAST "abspath",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.abs_path
		}, {
			.name = NULL
		}
	}
};

void xplCmdGetDocumentFilenameEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdGetDocumentFilenameParamsPtr params = (xplCmdGetDocumentFilenameParamsPtr) commandInfo->params;
	xplDocumentPtr doc;
	xmlNodePtr ret;
	xmlChar *fn;

	if (!(doc = params->document))
		doc = commandInfo->document;
	if (params->abs_path)
		fn = commandInfo->document->filename;
	else
		fn = commandInfo->document->filename + xmlStrlen(xplGetDocRoot()); // TODO relative path!
	if (!fn)
		fn = BAD_CAST "<unknown>";
	ret = xmlNewDocText(commandInfo->element->doc, fn);
	ASSIGN_RESULT(ret, false, true);
}
