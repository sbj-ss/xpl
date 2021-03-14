#include <libxpl/xplcore.h>
#include <libxpl/xplmacro.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplparams.h>
#include <libxpl/xplsession.h>
#include <libxpl/xpltree.h>

void xplCmdIsolateEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdIsolateParams
{
	bool repeat;
	bool share_session;
	bool inherit_macros;
	bool parallel;
	bool delay_start;
} xplCmdIsolateParams, *xplCmdIsolateParamsPtr;

static const xplCmdIsolateParams params_stencil =
{
	.repeat = false,
	.share_session = false,
	.inherit_macros = false,
	.parallel = false,
	.delay_start = false
};

xplCommand xplIsolateCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdIsolateEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdIsolateParams),
	.parameters = {
		{
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		},
		{
			.name = BAD_CAST "sharesession",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.share_session
		}, {
			.name = BAD_CAST "inheritmacros",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.inherit_macros
		}, {
			.name = BAD_CAST "parallel",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.parallel
		}, {
			.name = BAD_CAST "delaystart",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.delay_start
		}, {
			.name = NULL
		}
	}
};

void xplCmdIsolateEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdIsolateParamsPtr params = (xplCmdIsolateParamsPtr) commandInfo->params;
	xplDocumentPtr child;
	xplError status;
	xmlNodePtr content;
#ifdef _THREADING_SUPPORT
	xmlNodeSetPtr landing_point_path;
#endif

#ifndef _THREADING_SUPPORT
	if (params->parallel)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "no threading support compiled in"), true, true);
		return;
	}
#endif
	if (!params->parallel && params->delay_start)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "delaystart can only be used for parallel execution"), true, true);
		return;
	}
	if (!commandInfo->element->children)
	{
		ASSIGN_RESULT(NULL, false, true);
		return;
	}
#ifdef _THREADING_SUPPORT
	if (params->parallel && !(landing_point_path = xplGetNodeAncestorOrSelfAxis(commandInfo->element)))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "out of memory"), true, true);
		return;
	}
#endif
	if (!(child = xplDocumentCreateChild(commandInfo->document, commandInfo->element, params->inherit_macros, params->share_session)))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "out of memory"), true, true);
		return;
	}
	if (params->parallel)
	{
#ifdef _THREADING_SUPPORT
		child->landing_point_path = landing_point_path;
		child->async = true;
		if (xplStartChildThread(commandInfo->document, child, !params->delay_start))
		{
			commandInfo->element->ns = NULL; /* don't process the command multiple times */
			ASSIGN_RESULT(NULL, false, false);
		} else {
			xplDocumentFree(child);
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "couldn't spawn child thread"), true, true);
		}
		/* we don't need #else here - this was handled above */
#endif
	} else {
		status = xplDocumentApply(child);
		if (status == XPL_ERR_NO_ERROR || status == XPL_ERR_FATAL_CALLED)
		{
			if ((status == XPL_ERR_FATAL_CALLED) && cfgWarnOnFatalErrorsInIsolatedDocuments)
				xplDisplayWarning(commandInfo->element, BAD_CAST ":fatal called while processing child document");
			xplMergeDocOldNamespaces(child->document, commandInfo->element->doc);
			content = xplDetachChildren((xmlNodePtr) child->document);
			xmlSetListDoc(content, commandInfo->element->doc);
			xplSetChildren(commandInfo->element, content);
			/* xplLiftNsDefs() removes duplicated nsDefs instead of lifting them */
			xplLiftNsDefs(commandInfo->element->parent, content, content->children);
			ASSIGN_RESULT(xplDetachChildren(commandInfo->element->children), params->repeat, true);
		} else
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "error \"%s\" processing child document", xplErrorToString(status)), true, true);
		if (params->share_session)
			child->shared_session = NULL;
		else if (child->shared_session)
			xplSessionFreeLocal(child->shared_session);
		xplParamsFree(child->params);
		xplDocumentFree(child);
	}
}
