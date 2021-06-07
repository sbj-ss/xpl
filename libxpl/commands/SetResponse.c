#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdSetResponseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdSetResponseParams
{
	bool has_content;
} xplCmdSetResponseParams, *xplCmdSetResponseParamsPtr;

static const xplCmdSetResponseParams params_stencil =
{
	.has_content = false
};

xplCommand xplSetResponseCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdSetResponseEpilogue,
	.flags = XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE | XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.stencil_size = sizeof(xplCmdSetResponseParams),
	.params_stencil = &params_stencil,
	.parameters = {
		{
			.name = BAD_CAST "hascontent",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.has_content
		}, {
			.name = NULL
		}
	}
};

void xplCmdSetResponseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSetResponseParamsPtr params = (xplCmdSetResponseParamsPtr) commandInfo->params;

	if (commandInfo->document->response)
		XPL_FREE(commandInfo->document->response);
	commandInfo->document->response = commandInfo->content;
	commandInfo->content = NULL;
	commandInfo->document->has_meaningful_content = params->has_content;
	ASSIGN_RESULT(NULL, false, true);
}
