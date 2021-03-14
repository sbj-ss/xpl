#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>

void xplCmdGetDocumentFilenameEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdGetDocumentFilenameParams
{
	bool abs_path;
} xplCmdGetDocumentFilenameParams, *xplCmdGetDocumentFilenameParamsPtr;

static const xplCmdGetDocumentFilenameParams params_stencil =
{
	.abs_path = false
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

	doc = commandInfo->document;
	if (params->abs_path)
		fn = xplFormatMessage(BAD_CAST "%s%s", doc->path, doc->filename);
	else
		fn = xplFormatMessage(BAD_CAST "%s%s", doc->path + xmlStrlen(xplGetDocRoot()) + 1, doc->filename);
	ret = xmlNewDocText(commandInfo->element->doc, NULL);
	ret->content = fn;
	ASSIGN_RESULT(ret, false, true);
}
