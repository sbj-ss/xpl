#include <libxpl/xplcore.h>
#include <libxpl/xpltree.h>
#include "commands/Container.h"

typedef struct _xplCmdContainerParams
{
	bool repeat;
	bool return_content;
} xplCmdContainerParams, *xplCmdContainerParamsPtr;

static const xplCmdContainerParams params_stencil =
{
	.repeat = false,
	.return_content = true
};

xplCommand xplContainerCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdContainerEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdContainerParams),
	.parameters = {
		{
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = BAD_CAST "returncontent",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.return_content
		}, {
			.name = NULL
		}
	}
};

void xplCmdContainerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdContainerParamsPtr params = (xplCmdContainerParamsPtr) commandInfo->params;

	if (params->return_content)
		ASSIGN_RESULT(xplDetachContent(commandInfo->element), params->repeat, true);
	else {
		if (commandInfo->element->children)
			xplDocDeferNodeListDeletion(commandInfo->document, xplDetachContent(commandInfo->element->children));
		ASSIGN_RESULT(NULL, false, true);
	}
}
