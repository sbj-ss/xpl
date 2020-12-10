#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>

void xplCmdFileExistsEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdFileExistsParams
{
	xmlChar *file;
	bool abs_path;
} xplCmdFileExistsParams, *xplCmdFileExistsParamsPtr;

static const xplCmdFileExistsParams params_stencil =
{
	.file = NULL,
	.abs_path = false
};

xplCommand xplFileExistsCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdFileExistsEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdFileExistsParams),
	.parameters = {
		{
			.name = BAD_CAST "file",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.file
		}, {
			.name = BAD_CAST "abspath",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.abs_path
		}, {
			.name = NULL
		}
	}
};

void xplCmdFileExistsEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
/* TODO check R/W access */
	xplCmdFileExistsParamsPtr params = (xplCmdFileExistsParamsPtr) commandInfo->params;
	xmlChar *filename = NULL, *value;
	xmlNodePtr ret;

	if (params->abs_path)
		filename = BAD_CAST XPL_STRDUP((char*) params->file);
	else
		filename = xplFullFilename(params->file, commandInfo->document->app_path);

	if (xprCheckFilePresence(filename))
		value = BAD_CAST "true";
	else
		value = BAD_CAST "false";
	ret = xmlNewDocText(commandInfo->document->document, value);
	ASSIGN_RESULT(ret, false, true);
}
