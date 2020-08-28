#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xpltree.h>
#include "commands/Assert.h"

typedef struct _xplCmdAssertParams
{
	xmlChar *message;
} xplCmdAssertParams, *xplCmdAssertParamsPtr;

static const xplCmdAssertParams params_stencil =
{
	.message = NULL
};

xplCommand xplAssertCommand =
{
	.prologue = xplCmdAssertPrologue,
	.epilogue = xplCmdAssertEpilogue,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdAssertParams),
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE | XPL_CMD_FLAG_CONTENT_FOR_EPILOGUE | XPL_CMD_FLAG_REQUIRE_CONTENT,
	.parameters = {
		{
			.name = BAD_CAST "message",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.value_stencil = &params_stencil.message
		}, {
			.name = NULL
		}
	}
};

void xplCmdAssertPrologue(xplCommandInfoPtr commandInfo)
{
	UNUSED_PARAM(commandInfo);
}

void xplCmdAssertEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdAssertParamsPtr cmd_params = (xplCmdAssertParamsPtr) commandInfo->params;
	xmlXPathObjectPtr ct = NULL;
	int smth = 0;

	if (!cfgEnableAssertions)
	{
		ASSIGN_RESULT(NULL, false, true);
		return;
	}
	ct = xplSelectNodes(commandInfo, commandInfo->element, commandInfo->content);
	if (ct)
	{
		switch(ct->type)
		{
		case XPATH_NODESET:
			smth = (ct->nodesetval)? ct->nodesetval->nodeNr: 0;
			break;
		case XPATH_BOOLEAN:
			smth = ct->boolval;
			break;
		case XPATH_NUMBER:
			smth = (ct->floatval != 0.0);
			break;
		case XPATH_STRING:
			smth = (ct->stringval && *ct->stringval);
			break;
		default:
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "unsupported XPath result type %d (expression is %s)", ct->type, commandInfo->content), true, true);
			goto done;
		}
		if (!smth)
		{
			if (cmd_params->message)
			{
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, cmd_params->message), true, true);
			} else {
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "assertion \"%s\" failed", commandInfo->content), true, true);
			}
		} else {
			ASSIGN_RESULT(NULL, false, true);
		}
	} else {
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "invalid XPath expression (%s)", commandInfo->content), true, true);
	}
done:
	if (ct)
		xmlXPathFreeObject(ct);
}
