#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>

void xplCmdValueOfEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdValueOfParams
{
	xmlXPathObjectPtr select;
} xplCmdValueOfParams, *xplCmdValueOfParamsPtr;

static const xplCmdValueOfParams params_stencil =
{
	.select = NULL
};

xplCommand xplValueOfCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdValueOfEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdValueOfParams),
	.parameters = {
		{
			.name = BAD_CAST "select",
			.type = XPL_CMD_PARAM_TYPE_XPATH,
			.required = true,
			.extra.xpath_type = XPL_CMD_PARAM_XPATH_TYPE_SCALAR,
			.value_stencil = &params_stencil.select
		}, {
			.name = NULL
		}
	}
};

void xplCmdValueOfEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdValueOfParamsPtr params = (xplCmdValueOfParamsPtr) commandInfo->params;
	xmlNodePtr ret;
	
	ret = xmlNewDocText(commandInfo->document->document, NULL);
	ret->content = xmlXPathCastToString(params->select);
	ASSIGN_RESULT(ret, false, true);
}
