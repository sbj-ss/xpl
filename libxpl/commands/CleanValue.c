#include <libxpl/xplmessages.h>
#include <libxpl/xplparams.h>
#include <libxpl/xpltree.h>

void xplCmdCleanValueEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef enum _xplCmdCleanBehavior
{
	CLEAN_BEHAVIOR_EXTRACT,
	CLEAN_BEHAVIOR_CLEAR,
	CLEAN_BEHAVIOR_COMPLAIN
} xplCmdCleanBehavior;

typedef struct _xplCmdCleanValueParams
{
	xplExpectType expect;
	xplCmdCleanBehavior behavior;
} xplCmdCleanValueParams, *xplCmdCleanValueParamsPtr;

static const xplCmdCleanValueParams params_stencil =
{
	.expect = XPL_EXPECT_UNSPECIFIED,
	.behavior = CLEAN_BEHAVIOR_COMPLAIN
};

static xplCmdParamDictValue behavior_values[] =
{
	{ BAD_CAST "extract", CLEAN_BEHAVIOR_EXTRACT },
	{ BAD_CAST "clear", CLEAN_BEHAVIOR_CLEAR },
	{ BAD_CAST "complain", CLEAN_BEHAVIOR_COMPLAIN },
	{ NULL, 0 }
};

xplCommand xplCleanValueCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdCleanValueEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdCleanValueParams),
	.parameters = {
		{
			.name = BAD_CAST "expect",
			.type = XPL_CMD_PARAM_TYPE_INT_CUSTOM_GETTER,
			.extra.int_getter = xplExpectTypeGetter,
			.value_stencil = &params_stencil.expect
		}, {
			.name = BAD_CAST "behavior",
			.type = XPL_CMD_PARAM_TYPE_DICT,
			.extra.dict_values = behavior_values,
			.value_stencil = &params_stencil.behavior
		}, {
			.name = NULL
		}
	}
};

void xplCmdCleanValueEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdCleanValueParamsPtr params = (xplCmdCleanValueParamsPtr) commandInfo->params;
	xmlChar *clean_value;
	xmlNodePtr ret = NULL;

	if (!commandInfo->content)
	{
		ASSIGN_RESULT(NULL, false, true);
		return;
	}
	if ((clean_value = xplCleanTextValue(commandInfo->content, params->expect)))
	{
		if (xmlStrcmp(commandInfo->content, clean_value))
		{
			switch (params->behavior)
			{
				case CLEAN_BEHAVIOR_EXTRACT:
					ret = xmlNewDocText(commandInfo->element->doc, NULL);
					ret->content = clean_value;
					break;
				case CLEAN_BEHAVIOR_CLEAR:
					XPL_FREE(clean_value);
					break;
				case CLEAN_BEHAVIOR_COMPLAIN:
					ret = xplCreateErrorNode(commandInfo->element, "value '%s' doesn't pass validation", commandInfo->content);
					XPL_FREE(clean_value);
					break;
				default:
					DISPLAY_INTERNAL_ERROR_MESSAGE();
			}
		} else {
			ret = xmlNewDocText(commandInfo->element->doc, NULL);
			ret->content = clean_value;
		}
		ASSIGN_RESULT(ret, false, true);
	} else
		ASSIGN_RESULT(NULL, false, true);
}
