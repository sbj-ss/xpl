#include <libxpl/xplcore.h>
#include <libxpl/xplmacro.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplparams.h>
#include <libxpl/xplsession.h>
#include <libxpl/xpltree.h>
#include "commands/Isolate.h"

typedef struct _xplCmdIsolateParams
{
	bool share_session;
	bool inherit_macros;
	bool parallel;
	bool delay_start;
} xplCmdIsolateParams, *xplCmdIsolateParamsPtr;

static const xplCmdIsolateParams params_stencil =
{
	.share_session = false,
	.inherit_macros = false,
	.parallel = false,
	.delay_start = false
};

xplCommand xplIsolateCommand =
{
	.prologue = xplCmdIsolatePrologue,
	.epilogue = xplCmdIsolateEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdIsolateParams),
	.parameters = {
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

static xplDocumentPtr _createChildDoc(xplDocumentPtr doc, xmlNodePtr carrier, bool inherit_macros, xplParamsPtr env, xplSessionPtr session)
{
	xmlNodePtr content, root;
	xmlHashTablePtr macros = NULL;
	xplDocumentPtr ret;

	content = xplDetachContent(carrier);
	root = xmlNewDocNode(carrier->doc, NULL, BAD_CAST "Root", NULL);
	if (inherit_macros)
		macros = xplCloneMacroTableUpwards(carrier, root);
	root->_private = macros;
	xplSetChildren(root, content);
	xplSetChildren(carrier, root);
	xplMakeNsSelfContainedTree(root);
	xplSetChildren(carrier, NULL);
	ret = xplDocumentInit(doc->app_path, env, session);
	ret->prologue = doc->prologue;
	ret->main = ret;
	ret->epilogue = doc->epilogue;
	ret->source = doc->source;
	ret->status = doc->status;
	ret->role = doc->role;
	ret->parent = doc;
	ret->document = xmlNewDoc(BAD_CAST "1.0");
	xplSetChildren((xmlNodePtr) ret->document, root);
	xmlSetListDoc(root, ret->document);
	if (doc->filename)
	{
		ret->filename = BAD_CAST XPL_STRDUP((char*) doc->filename);
		ret->document->URL = (xmlChar*) XPL_MALLOC((size_t) xmlStrlen(doc->filename) + 8);
		sprintf((char*) ret->document->URL, "[FORK] %s", doc->filename);
	} else
		ret->document->URL = BAD_CAST XPL_STRDUP("[FORK]");
	return ret;
}

void xplCmdIsolatePrologue(xplCommandInfoPtr commandInfo)
{
	UNUSED_PARAM(commandInfo);
}

void xplCmdIsolateEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdIsolateParamsPtr params = (xplCmdIsolateParamsPtr) commandInfo->params;
	xplDocumentPtr child;
	xplParamsPtr env;
	xplSessionPtr session;
	xplError status;
	xmlNodePtr content;

#ifndef _THREADING_SUPPORT
	if (params->parallel)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "no threading support compiled in"), true, true);
		return;
	}
#endif
	if (params->parallel && !commandInfo->element->children)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "can't isolate: command content is empty"), true, true);
		return;
	}
	env = xplParamsCopy(commandInfo->document->environment);
	session = params->share_session? commandInfo->document->session: NULL;
	if (!(child = _createChildDoc(commandInfo->document, commandInfo->element, params->inherit_macros, env, session)))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "out of memory"), true, true);
		return;
	}
	if (params->parallel)
	{
#ifdef _THREADING_SUPPORT
		child->landing_point = commandInfo->element;
		xplEnsureDocThreadSupport(commandInfo->document);
		if (xplStartChildThread(commandInfo->document, child, !delaystart))
			ASSIGN_RESULT(NULL, false, false);
		else
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "couldn't spawn child thread"), true, true);
		/* we don't need #else here - this was handled above */
#endif
	} else {
		if ((status = xplDocumentApply(child)) == XPL_ERR_NO_ERROR)
		{
			/* TODO don't clone when redundant namespace removal code is ready
			content = detachContent(child->document->children);
			xmlSetListDoc(content, commandInfo->element->doc);
			irredundant...()
			*/
			content = xplCloneNodeList(child->document->children->children, commandInfo->element, commandInfo->element->doc);
			xplDownshiftNodeNsDef(content, commandInfo->element->nsDef);
		} else
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "error \"%s\" processing child document", xplErrorToString(status)), true, true);
		xplDocumentFree(child);
		xplParamsFree(env);
		ASSIGN_RESULT(content, false, true);
	}
}
