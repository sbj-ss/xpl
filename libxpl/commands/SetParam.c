#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplparams.h>
#include <libxpl/xpltree.h>

void xplCmdSetParamEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef enum _xplCmdSetParamMode
{
	SET_PARAM_MODE_ADD,
	SET_PARAM_MODE_REPLACE
} xplCmdSetParamMode;

typedef struct _xplCmdSetParamParams
{
	xmlChar *name;
	xplCmdSetParamMode mode;
} xplCmdSetParamParams, *xplCmdSetParamParamsPtr;

static const xplCmdSetParamParams params_stencil =
{
	.name = NULL,
	.mode = SET_PARAM_MODE_REPLACE
};

xplCmdParamDictValue mode_dict[] =
{
	{ .name = BAD_CAST "add", .value = SET_PARAM_MODE_ADD },
	{ .name = BAD_CAST "replace", .value = SET_PARAM_MODE_REPLACE },
	{ .name = NULL }
};

xplCommand xplSetParamCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdSetParamEpilogue,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdSetParamParams),
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE,
	.parameters = {
		{
			.name = BAD_CAST "name",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.required = true,
			.value_stencil = &params_stencil.name
		}, {
			.name = BAD_CAST "mode",
			.type = XPL_CMD_PARAM_TYPE_DICT,
			.extra.dict_values = mode_dict,
			.value_stencil = &params_stencil.mode
		}, {
			.name = NULL
		}
	}
};

void xplCmdSetParamEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdSetParamParamsPtr params = (xplCmdSetParamParamsPtr) commandInfo->params;
	xplParamResult res;
	xmlNodePtr err_node = NULL;

	ASSIGN_RESULT(NULL, false, true);
	if (!commandInfo->document->params)
		return;
	switch (params->mode)
	{
		case SET_PARAM_MODE_ADD:
			res = xplParamAddValue(commandInfo->document->params, params->name, commandInfo->content, XPL_PARAM_TYPE_USERDATA);
			break;
		case SET_PARAM_MODE_REPLACE:
			res = xplParamReplaceValue(commandInfo->document->params, params->name, commandInfo->content, XPL_PARAM_TYPE_USERDATA);
			break;
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
			return;
	}
	switch (res)
	{
		case XPL_PARAM_RES_OK:
			commandInfo->content = NULL;
			break;
		case XPL_PARAM_RES_OUT_OF_MEMORY:
			err_node = xplCreateErrorNode(commandInfo->element, BAD_CAST "out of memory");
			break;
		case XPL_PARAM_RES_INVALID_INPUT:
			err_node = xplCreateErrorNode(commandInfo->element, BAD_CAST "internal interpreter error, possibly out of memory");
			break;
		case XPL_PARAM_RES_TYPE_CLASH:
			err_node = xplCreateErrorNode(commandInfo->element, BAD_CAST "parameter with the same name (%s), but of another type already exists", params->name);
			break;
		case XPL_PARAM_RES_READ_ONLY:
			err_node = xplCreateErrorNode(commandInfo->element, BAD_CAST "parameter \"%s\" is read-only", params->name);
			break;
		case XPL_PARAM_RES_INTERNAL_ERROR:
			err_node = xplCreateErrorNode(commandInfo->element, BAD_CAST "internal hash error, possibly out of memory");
			break;
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
			err_node = xplCreateErrorNode(commandInfo->element, BAD_CAST "internal error");
	}
	if (err_node)
		ASSIGN_RESULT(err_node, true, true);
}
