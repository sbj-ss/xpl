#include <libxpl/xplcommand.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include <libxpl/abstraction/xef.h>

void xplCmdDBSessionPrologue(xplCommandInfoPtr commandInfo);
void xplCmdDBSessionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdDBSessionParams
{
	bool repeat;
	xmlChar *dbname;
} xplCmdDBSessionParams, *xplCmdDBSessionParamsPtr;

static const xplCmdDBSessionParams params_stencil =
{
	.repeat = false,
	.dbname = NULL
};

xplCommand xplDBSessionCommand =
{
	.prologue = xplCmdDBSessionPrologue,
	.epilogue = xplCmdDBSessionEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_PROLOGUE | XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdDBSessionParams),
	.parameters = {
		{
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = BAD_CAST "dbname",
			.type = XPL_CMD_PARAM_TYPE_STRING,
			.required = true,
			.value_stencil = &params_stencil.dbname
		}, {
			.name = NULL
		}
	}
};

void xplCmdDBSessionPrologue(xplCommandInfoPtr commandInfo)
{
#ifdef _XEF_HAS_DB
	xplCmdDBSessionParamsPtr params = (xplCmdDBSessionParamsPtr) commandInfo->params;
	xplDBListPtr db_list;
	xplDBPtr db;
	xmlChar *error_text;

	if (!(db_list = xplLocateDBList(params->dbname)))
	{
		commandInfo->prologue_error = xplCreateErrorNode(commandInfo->element, "unknown database \"%s\"", params->dbname);
		xplDocDeferNodeListDeletion(commandInfo->document, xplDetachChildren(commandInfo->element));
		return;
	}
	if (!(db = xplGetOrCreateDB(db_list, xefDbEstablishConnection, &error_text)))
	{
		commandInfo->prologue_error = xplCreateErrorNode(commandInfo->element, "cannot connect to database \"%s\": %s", params->dbname, error_text);
		xplDocDeferNodeListDeletion(commandInfo->document, xplDetachChildren(commandInfo->element));
		XPL_FREE(error_text);
		return;
	}
	commandInfo->prologue_state = commandInfo->document->cur_db;
	commandInfo->document->cur_db = db;
#else
	commandInfo->prologue_error = xplCreateErrorNode(commandInfo->element, "database support not compiled in");
	xplDocDeferNodeListDeletion(commandInfo->document, xplDetachChildren(commandInfo->element));
#endif
}

void xplCmdDBSessionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdDBSessionParamsPtr params = (xplCmdDBSessionParamsPtr) commandInfo->params;
#ifdef _XEF_HAS_DB
	xplDBPtr db = (xplDBPtr) commandInfo->document->cur_db;

	xplReleaseDB(db);
	commandInfo->document->cur_db = (xplDBPtr) commandInfo->prologue_state;
#endif
	if (commandInfo->prologue_error)
		ASSIGN_RESULT(commandInfo->prologue_error, true, true);
	else
		ASSIGN_RESULT(xplDetachChildren(commandInfo->element), params->repeat, true);
}
