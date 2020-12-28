#include <libxpl/xplcore.h>
#include <libxpl/xpltree.h>

void xplCmdTextPrologue(xplCommandInfoPtr commandInfo);
void xplCmdTextEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdTextParams
{
	bool repeat;
} xplCmdTextParams, *xplCmdTextParamsPtr;

static const xplCmdTextParams params_stencil =
{
	.repeat = false
};

xplCommand xplTextCommand = {
	.prologue = NULL,
	.epilogue = xplCmdTextEpilogue,
	.flags = XPL_CMD_FLAG_CONTENT_SAFE | XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdTextParams),
	.parameters = {
		{
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};

void xplCmdTextPrologue(xplCommandInfoPtr commandInfo)
{
	commandInfo->document->indent_spinlock++;
}

void xplCmdTextEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdTextParamsPtr params = (xplCmdTextParamsPtr) commandInfo->params;

	commandInfo->document->indent_spinlock--;
	if (!(commandInfo->element->type & XPL_NODE_DELETION_MASK))
		ASSIGN_RESULT(xplDetachContent(commandInfo->element), params->repeat, true);
	else
		ASSIGN_RESULT(NULL, false, false);
}
