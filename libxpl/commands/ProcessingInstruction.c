#include <libxpl/xplcommand.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdProcessingInstructionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdProsessingInstructionParams
{
	xmlChar *name;
} xplCmdProcessingInstructionParams, *xplCmdProcessingInstructionParamsPtr;

static const xplCmdProcessingInstructionParams params_stencil =
{
	.name = NULL
};

xplCommand xplProcessingInstructionCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdProcessingInstructionEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE | XPL_CMD_FLAG_REQUIRE_CONTENT,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdProcessingInstructionParams),
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.required = true,
			.value_stencil = &params_stencil.name
		}, {
			.name = NULL
		}
	}
};

void xplCmdProcessingInstructionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdProcessingInstructionParamsPtr params = (xplCmdProcessingInstructionParamsPtr) commandInfo->params;
	xmlNodePtr ret;

	ret = xmlNewDocPI(commandInfo->element->doc, params->name, NULL);
	ret->content = commandInfo->content;
	commandInfo->content = NULL;
	ASSIGN_RESULT(ret, false, true);
}
