#include <libxpl/xplcore.h>
#include <libxpl/xplmacro.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplparams.h>
#include <libxpl/xplsession.h>
#include <libxpl/xplutils.h>
#include "commands/Isolate.h"

void xplCmdIsolatePrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdIsolateEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define SHARESESSION_ATTR (BAD_CAST "sharesession")
#define INHERITMACROS_ATTR (BAD_CAST "inheritmacros")
#define PARALLEL_ATTR (BAD_CAST "parallel")
#define DELAYSTART_ATTR (BAD_CAST "delaystart")
	bool sharesession;
	bool parallel;
	bool inheritmacros;
	bool delaystart;
	xmlNodePtr content, root, error;
	xplDocumentPtr child;
	xplParamsPtr env;
	xplSessionPtr session;
	xplError status;
	xmlHashTablePtr macros = NULL;

	if ((error = xplDecodeCmdBoolParam(commandInfo->element, SHARESESSION_ATTR, &sharesession, false)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, INHERITMACROS_ATTR, &inheritmacros, false)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, PARALLEL_ATTR, &parallel, false)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
#ifndef _THREADING_SUPPORT
	if (parallel)
	{
		error = xplCreateErrorNode(commandInfo->element, BAD_CAST "no threading support compiled in");
		ASSIGN_RESULT(error, true, true);
		return;
	}
#endif
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, DELAYSTART_ATTR, &delaystart, false)))
	{
		ASSIGN_RESULT(error, true, true);
		return;
	}
	content = detachContent(commandInfo->element);
	if (!content)
	{
		if (parallel)
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "command content is empty"), true, true);
		goto done;
	}
	root = xmlNewDocNode(commandInfo->element->doc, NULL, BAD_CAST "Root", NULL);
	if (inheritmacros)
		macros = xplCloneMacroTableUpwards(commandInfo->element, root);
	root->_private = macros;
	setChildren(root, content);
	setChildren(commandInfo->element, root);
	makeNsIndepTree(root);
	setChildren(commandInfo->element, NULL);
	env = xplParamsCopy(commandInfo->document->environment);
	session = sharesession? commandInfo->document->session: NULL;
	child = xplDocumentInit(commandInfo->document->app_path, env, session);
	child->prologue = commandInfo->document->prologue;
	child->main = child;
	child->epilogue = commandInfo->document->epilogue;
	child->source = commandInfo->document->source;
	child->status = commandInfo->document->status;
	child->role = commandInfo->document->role;
	child->parent = commandInfo->document;
	child->document = xmlNewDoc(BAD_CAST "1.0");
	// child->document->children = child->document->last = root;
	setChildren((xmlNodePtr) child->document, root);
	xmlSetListDoc(root, child->document);
	if (commandInfo->document->filename)
	{
		child->filename = xmlStrdup(commandInfo->document->filename);
		child->document->URL = (xmlChar*) xmlMalloc((size_t) xmlStrlen(commandInfo->document->filename) + 8);
		sprintf((char*) child->document->URL, "[FORK] %s", commandInfo->document->filename);
	} else
		child->document->URL = xmlStrdup(BAD_CAST "[FORK]");
	if (parallel)
	{
#ifdef _THREADING_SUPPORT
		child->landing_point = commandInfo->element;
		xplEnsureDocThreadSupport(commandInfo->document);
		if (xplStartChildThread(commandInfo->document, child, !delaystart))
			ASSIGN_RESULT(NULL, false, false);
		else
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "couldn't spawn child thread"), true, true);
#endif
	} else {
		if ((status = xplDocumentApply(child)) != XPL_ERR_NO_ERROR)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "error \"%s\" processing child document", 
				xplErrorToString(status)), true, true);
			goto done;
		}
		/* TODO don't clone when redundant namespace removal code is ready
		content = detachContent(child->document->children);
		xmlSetListDoc(content, commandInfo->element->doc);
		irredundant...()
		*/
		content = cloneNodeList(child->document->children->children, commandInfo->element, commandInfo->element->doc);
		downshiftNodeNsDef(content, commandInfo->element->nsDef);
		//child->document->intSubset = NULL;
		xplDocumentFree(child);
		xplParamsFree(env);
		ASSIGN_RESULT(content, false, true);
	}
done:
	;
}

xplCommand xplIsolateCommand = { xplCmdIsolatePrologue, xplCmdIsolateEpilogue };
