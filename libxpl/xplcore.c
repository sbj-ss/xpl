#include "Configuration.h"
#include <stdio.h>
#include <string.h>
#include <libxml/xmlsave.h>
#include <libxml/xpathInternals.h>
#include <libxpl/abstraction/xef.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplsave.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>
#include <libxpl/xplversion.h>
#include "Register.h"

/* internal statics */
static bool parser_loaded = 0;
static xplGetAppTypeFunc get_app_type_func = NULL;
volatile int active_thread_count = 0;
static XPR_MUTEX global_conf_mutex;

const xmlChar* xplErrorToString(xplError error)
{
	switch (error)
	{
	case XPL_ERR_FATAL_CALLED:
		return BAD_CAST "xpl:fatal was called";
	case XPL_ERR_NO_ERROR:
		return BAD_CAST "no error";
	case XPL_ERR_INVALID_DOCUMENT:
		return BAD_CAST "invalid input document";
	case XPL_ERR_DOC_NOT_CREATED:
		return BAD_CAST "couldn't parse document";
	case XPL_ERR_NO_PARSER:
		return BAD_CAST "parser couldn't be initialized (not enough memory?..)";
	case XPL_ERR_INVALID_INPUT:
		return BAD_CAST "invalid parameters";
	case XPL_ERR_NOT_YET_REACHED:
		return BAD_CAST "execution point not yet reached";
	default:
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return BAD_CAST "unknown error";
	}
}

const xmlChar* xplErrorToShortString(xplError error)
{
	switch (error)
	{
	case XPL_ERR_FATAL_CALLED:
		return BAD_CAST "fatal";
	case XPL_ERR_NO_ERROR:
		return BAD_CAST "ok";
	case XPL_ERR_INVALID_DOCUMENT:
		return BAD_CAST "invalid_doc";
	case XPL_ERR_DOC_NOT_CREATED:
		return BAD_CAST "malformed_doc";
	case XPL_ERR_NO_PARSER:
		return BAD_CAST "no_parser";
	case XPL_ERR_INVALID_INPUT:
		return BAD_CAST "invalid_params";
	case XPL_ERR_NOT_YET_REACHED:
		return BAD_CAST "not_reached";
	default:
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return BAD_CAST "unknown";
	}
}

/* common document initialization part */
static void _xpathDummyError(void *data, xmlErrorPtr error)
{
	UNUSED_PARAM(data);
	UNUSED_PARAM(error);
}

xplDocumentPtr xplDocumentInit(xmlChar *aDocRoot, xplParamsPtr aParams, xplSessionPtr aSession)
{
	size_t doc_root_len;
	xplDocumentPtr doc;
	xmlChar delim;
	
	doc = (xplDocumentPtr) XPL_MALLOC(sizeof(xplDocument));
	if (!doc)
		return NULL;
	memset(doc, 0, sizeof(xplDocument));

	if (!aDocRoot)
		aDocRoot = cfgDocRoot;
	if (aDocRoot && *aDocRoot)
	{
		doc_root_len = xmlStrlen(aDocRoot);
		delim = 0;
		if (aDocRoot[doc_root_len - 1] == XPR_PATH_INVERSE_DELIM)
			delim = XPR_PATH_DELIM;
		else if (aDocRoot[doc_root_len - 1] != XPR_PATH_DELIM)
			doc_root_len++;
		doc->doc_root = (xmlChar*) XPL_MALLOC(doc_root_len + 1);
		if (!doc->doc_root) 
		{
			xplDocumentFree(doc);
			return NULL;
		}
		strcpy((char*) doc->doc_root, (const char*) aDocRoot);
		if (delim)
			doc->doc_root[doc_root_len - 1] = delim;
		else if (doc->doc_root[doc_root_len - 1] != XPR_PATH_DELIM) {
			doc->doc_root[doc_root_len - 1] = XPR_PATH_DELIM;
			doc->doc_root[doc_root_len] = 0;
		}
	} else {
		doc->doc_root = NULL;
	}
	doc->xpath_ctxt = xmlXPathNewContext(NULL);
	if (!doc->xpath_ctxt)
	{
		xplDocumentFree(doc);
		return NULL;
	}
	doc->xpath_ctxt->error = _xpathDummyError;
	doc->expand = true;
	doc->shared_session = aSession;
	doc->params = aParams;
	doc->status = XPL_ERR_NOT_YET_REACHED;
	return doc;
}

xplDocumentPtr xplDocumentCreateFromFile(xmlChar *aDocRoot, xmlChar *aFilename, xplParamsPtr aParams, xplSessionPtr aSession)
{
	xplDocumentPtr ret;	
	size_t path_len;
	size_t max_fn_len;

	if (!aFilename)
		return NULL;
	ret = xplDocumentInit(aDocRoot, aParams, aSession);
	path_len = ret->doc_root? xmlStrlen(ret->doc_root): 0;
	max_fn_len = path_len + xmlStrlen(aFilename) + 1;
	ret->filename = (xmlChar*) XPL_MALLOC(max_fn_len);
	if (!ret->filename)
	{
		xplDocumentFree(ret);
		return NULL;
	}
	if (ret->doc_root)
		strcpy((char*) ret->filename, (const char*) ret->doc_root);
	strcpy((char*) ret->filename + path_len, (const char*) aFilename);

	if (!xprCheckFilePresence(ret->filename, false))
	{
		ret->error = xplFormatMessage(BAD_CAST "file '%s' not found", ret->filename);
		return ret;
	}
	ret->document = xmlParseFile((const char*) ret->filename);
	if (ret->document)
		ret->error = NULL;
	else {
		ret->error = xstrGetLastLibxmlError();
		xmlResetLastError();
	}

	return ret;
}

xplDocumentPtr xplDocumentCreateFromMemory(xmlChar* aDocRoot, xmlChar *aOrigin,  xplParamsPtr aParams, xplSessionPtr aSession, xmlChar *encoding)
{
	xplDocumentPtr ret = xplDocumentInit(aDocRoot, aParams, aSession);

	ret->origin = aOrigin;
	ret->document = xmlReadMemory((const char*) aOrigin, xmlStrlen(aOrigin), NULL, (const char*) encoding, 0);
	if (ret->document)
	{
		ret->error = NULL;
	} else {
		ret->error = xstrGetLastLibxmlError();
		xmlResetLastError();
	}

	return ret;
}
void xplDocumentFree(xplDocumentPtr doc)
{
	if (!doc)
		return;
#ifdef _THREADING_SUPPORT
	xplFinalizeDocThreads(doc, false);
#endif
	if (doc->doc_root)
		XPL_FREE(doc->doc_root);
	if (doc->filename)
		XPL_FREE(doc->filename);
	if (doc->error)
		XPL_FREE(doc->error);
	if (doc->async && doc->params)
		xplParamsFree(doc->params);
	if (doc->local_session)
		xplSessionFreeLocal(doc->local_session);
	if (doc->xpath_ctxt)
		xmlXPathFreeContext(doc->xpath_ctxt);
	if (doc->response)
		XPL_FREE(doc->response);
	xplClearDocStack(doc);
	if (doc->landing_point_path)
		xmlXPathFreeNodeSet(doc->landing_point_path);
	if (doc->deleted_nodes)
	{
		xplDeleteDeferredNodes(doc->deleted_nodes);
		xmlBufferFree(doc->deleted_nodes);
	}
	if (doc->document)
		xmlFreeDoc(doc->document);
	XPL_FREE(doc);
}

#ifdef _THREADING_SUPPORT
bool xplEnsureDocThreadSupport(xplDocumentPtr doc)
{
	if (doc->thread_handles)
		return true;
	if (!xprMutexInit(&doc->thread_landing_lock))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return false;
	}
	if (!xprMutexAcquire(&doc->thread_landing_lock))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		xplFinalizeDocThreads(doc, true);
		return false;
	}
	if (!(doc->thread_handles = xmlBufferCreateSize(2*sizeof(XPR_THREAD_HANDLE))))
	{
		xplFinalizeDocThreads(doc, true);
		return false;
	}
	xmlBufferSetAllocationScheme(doc->thread_handles, XML_BUFFER_ALLOC_HYBRID);
	if (!(doc->suspended_thread_docs = xmlBufferCreateSize(2*sizeof(xplDocumentPtr))))
	{
		xplFinalizeDocThreads(doc, true);
		return false;
	}
	xmlBufferSetAllocationScheme(doc->suspended_thread_docs, XML_BUFFER_ALLOC_HYBRID);
	return true;
}

void xplWaitForChildThreads(xplDocumentPtr doc)
{
	XPR_THREAD_HANDLE *handles;
	size_t count;

	if (!doc->thread_handles)
		return;
	handles = (XPR_THREAD_HANDLE*) xmlBufferContent(doc->thread_handles);
	count = xmlBufferLength(doc->thread_handles) / sizeof(XPR_THREAD_HANDLE);
	if (count)
	{
		SUCCEED_OR_DIE(xprMutexRelease(&doc->thread_landing_lock));
		xprWaitForThreads(handles, (int) count);
		SUCCEED_OR_DIE(xprMutexAcquire(&doc->thread_landing_lock));
	}
	xmlBufferEmpty(doc->thread_handles);
}

/* Thread wrapper */
static XPR_DECLARE_THREAD_ROUTINE(xplDocThreadWrapper, p)
{
	xplDocumentPtr doc = (xplDocumentPtr) p;
	xmlNodePtr content;
	xplError err;

	if (!doc)
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		xprExitThread((XPR_THREAD_RETVAL) -1);
	}
	err = xplDocumentApply(doc);
	if ((err != XPL_ERR_NO_ERROR) && (err != XPL_ERR_FATAL_CALLED))
		/* this shouldn't happen, but… */
		content = xplCreateErrorNode(doc->landing_point_path->nodeTab[0], BAD_CAST "error processing child document: \"%s\"", xplErrorToString(err));
	else {
		/* root element is a stub */
		xplMergeDocOldNamespaces(doc->document, doc->landing_point_path->nodeTab[0]->doc);
		content = xplDetachChildren(doc->document->children);
	}
	xmlSetListDoc(content, doc->parent->document);
	SUCCEED_OR_DIE(xprMutexAcquire(&doc->parent->thread_landing_lock));
	if (xplVerifyAncestorOrSelfAxis(xplFirstElementNode(doc->parent->document->children), doc->landing_point_path))
		xplDocDeferNodeDeletion(doc->parent, xplReplaceWithList(doc->landing_point_path->nodeTab[0], content));
	else {
		if (cfgWarnOnDeletedThreadLandingPoint)
			xplDisplayWarning(xplFirstElementNode(doc->document->children), BAD_CAST "thread landing point deleted");
		xmlFreeNodeList(content);
	}
	SUCCEED_OR_DIE(xprMutexRelease(&doc->parent->thread_landing_lock));
	xplDocumentFree(doc);
	xprExitThread((XPR_THREAD_RETVAL) 0);
	return (XPR_THREAD_RETVAL) 0; // make compiler happy
}

bool xplStartChildThread(xplDocumentPtr doc, xplDocumentPtr child, bool immediateStart)
{
	XPR_THREAD_HANDLE handle;

	if (!doc || !child) 
		return false;
	if (!xplEnsureDocThreadSupport(doc))
		return false;
	if (immediateStart)
	{
		if (xmlBufferGrow(doc->thread_handles, sizeof(XPR_THREAD_HANDLE)) < 0)
			return false;
		if (!(handle = xprStartThread(xplDocThreadWrapper, (XPR_THREAD_PARAM) child)))
			return false;
		return xmlBufferAdd(doc->thread_handles, BAD_CAST &handle, sizeof(XPR_THREAD_HANDLE)) == 0;
	} else
		return xmlBufferAdd(doc->suspended_thread_docs, BAD_CAST &child, sizeof(xplDocumentPtr)) == 0;
}

bool xplStartDelayedThreads(xplDocumentPtr doc)
{
	size_t pool_size, i;
	xplDocumentPtr *docs;
	bool ret = true;

	if (!doc)
		return false;
	if (!doc->suspended_thread_docs)
		return true;
	pool_size = xmlBufferLength(doc->suspended_thread_docs) / sizeof(xplDocumentPtr);
	for (i = 0, docs = (xplDocumentPtr*) xmlBufferContent(doc->suspended_thread_docs); i < pool_size; i++)
	{
		if (!xplStartChildThread(doc, docs[i], true)) // here we're almost sure that launch failed. delete the doc to prevent memory leak
		{
			ret = false;
			xplDocumentFree(docs[i]);
		}
	}
	xmlBufferEmpty(doc->suspended_thread_docs);
	return ret;
}

void xplDiscardSuspendedThreadDocs(xplDocumentPtr doc)
{
	size_t pool_size, i;
	xplDocumentPtr *docs;

	pool_size = xmlBufferLength(doc->suspended_thread_docs) / sizeof(xplDocumentPtr);
	if (pool_size && cfgWarnOnNeverLaunchedThreads)
		xplDisplayWarning(xplFirstElementNode(doc->document->children), BAD_CAST "%d suspended threads were never launched", pool_size);
	for (i = 0, docs = (xplDocumentPtr*) xmlBufferContent(doc->suspended_thread_docs); i < pool_size; i++)
		xplDocumentFree(docs[i]);
	xmlBufferEmpty(doc->suspended_thread_docs);
}

void xplFinalizeDocThreads(xplDocumentPtr doc, bool force_mutex_cleanup)
{
	bool has_thread_support = doc->thread_handles || doc->suspended_thread_docs;

	if (doc->thread_handles)
	{
		xplWaitForChildThreads(doc);
		xmlBufferFree(doc->thread_handles);
		doc->thread_handles = NULL;
	}
	if (doc->suspended_thread_docs)
	{
		xplDiscardSuspendedThreadDocs(doc);
		xmlBufferFree(doc->suspended_thread_docs);
		doc->suspended_thread_docs = NULL;
	}
	if (has_thread_support || force_mutex_cleanup)
	{
		xprMutexRelease(&doc->thread_landing_lock); // this may fail due to previous errors
		if (!xprMutexCleanup(&doc->thread_landing_lock))
			DISPLAY_INTERNAL_ERROR_MESSAGE();
	}
}
#endif

/* per-document node stack (for xpl:stack-*) */
void xplPushToDocStack(xplDocumentPtr doc, xmlNodePtr node)
{
	xmlNodePtr new_parent;

	if (!doc)
		return;
	new_parent = xmlNewDocNode(doc->document, NULL, BAD_CAST "carrier", NULL);
	xplSetChildren(new_parent, node->children);
	xplSetChildren(node, new_parent);
	xplMakeNsSelfContainedTree(new_parent);
	xplSetChildren(node, NULL);
	new_parent->prev = doc->stack;
	new_parent->parent = NULL;
	doc->stack = new_parent;
}

xmlNodePtr xplPopFromDocStack(xplDocumentPtr doc, xmlNodePtr parent)
{
	xmlNodePtr ret, carrier;

	if (!doc || !doc->stack)
		return NULL;
	carrier = doc->stack;
	doc->stack = carrier->prev;
	carrier->prev = NULL;
	ret = xplDetachChildren(carrier);
	xplLiftNsDefs(parent, carrier, ret);
	xmlFreeNode(carrier);
	return ret;
}

void xplClearDocStack(xplDocumentPtr doc)
{
	if (!doc || !doc->stack)
		return;
	while (doc->stack->prev)
		doc->stack = doc->stack->prev;
	xmlFreeNodeList(doc->stack);
	doc->stack = NULL;
}

bool xplDocStackIsEmpty(xplDocumentPtr doc)
{
	if (!doc)
		return true;
	if (!doc->stack)
		return true;
	return false;
}

/* Node deferred deletion */
void xplDeferNodeDeletion(xmlBufferPtr buf, xmlNodePtr cur)
{
	if (!buf || !cur)
		return;
	if ((int) cur->type & XPL_NODE_DELETION_DEFERRED_FLAG)
		return;
	xmlUnlinkNode(cur);
	xplMarkDOSAxisForDeletion(cur, XPL_NODE_DELETION_DEFERRED_FLAG, true);
	xmlBufferAdd(buf, BAD_CAST &cur, sizeof(xmlNodePtr)); // TODO possible OOM
}

void xplDeferNodeListDeletion(xmlBufferPtr buf, xmlNodePtr cur)
{
	xmlNodePtr next;

	if (!buf)
		return;
	while (cur)
	{
		next = cur->next;
		xplDeferNodeDeletion(buf, cur);
		cur = next;
	}
}

static bool _ensureDeletedNodesSupport(xplDocumentPtr doc)
{
	if (doc->deleted_nodes)
		return true;
	if (!(doc->deleted_nodes = xmlBufferCreateSize(8 * sizeof(xmlNodePtr))))
		return false;
	xmlBufferSetAllocationScheme(doc->deleted_nodes, XML_BUFFER_ALLOC_DOUBLEIT);
	return true;
}

void xplDocDeferNodeDeletion(xplDocumentPtr doc, xmlNodePtr cur)
{
	if (!doc || !cur)
		return;
	if (cfgFoolproofDestructiveCommands && doc->iterator_spin)
	{
		_ensureDeletedNodesSupport(doc);
		xplDeferNodeDeletion(doc->deleted_nodes, cur);
	} else {
		xmlUnlinkNode(cur);
		xmlFreeNode(cur);
	}
}

void xplDocDeferNodeListDeletion(xplDocumentPtr doc, xmlNodePtr cur)
{
	if (!doc || !cur)
		return;
	if (cfgFoolproofDestructiveCommands && doc->iterator_spin)
	{
		_ensureDeletedNodesSupport(doc);
		xplDeferNodeListDeletion(doc->deleted_nodes, cur);
	} else
		xmlFreeNodeList(cur);
}

void xplDeleteDeferredNodes(xmlBufferPtr buf)
{
	size_t i = 0;
	xmlNodePtr *p = (xmlNodePtr*) xmlBufferContent(buf);

	for (i = 0; i < xmlBufferLength(buf) / sizeof(xmlNodePtr); i++, p++)
	{
		xplMarkDOSAxisForDeletion(*p, XPL_NODE_DELETION_MASK, false);
		xmlUnlinkNode(*p);
		xmlFreeNode(*p);
	}
	xmlBufferEmpty(buf);
}

/* session wrappers */
static xmlNodePtr _selectAndCopyNodes(
	const xplDocumentPtr doc,
	const xmlNodePtr src,
	const xmlChar *select,
	const xmlNodePtr parent,
	bool *ok
)
{
	xmlXPathObjectPtr sel;
	xmlNodePtr cur, head = NULL, tail = NULL;
	size_t i;

	*ok = true;
	sel = xplSelectNodesWithCtxt(doc->xpath_ctxt, src, select);
	if (!sel)
	{
		*ok = false;
		return xplCreateErrorNode(parent, BAD_CAST "invalid select XPath expression '%s'", select);
	}
	if ((sel->type == XPATH_NODESET) && sel->nodesetval)
	{
		for (i = 0; i < (size_t) sel->nodesetval->nodeNr; i++)
		{
			cur = xplCloneAsNodeChild(sel->nodesetval->nodeTab[i], parent);
			APPEND_NODE_TO_LIST(head, tail, cur);
		}
	} else if (sel->type != XPATH_UNDEFINED) {
		head = xmlNewDocText(parent->doc, NULL);
		head->content = xmlXPathCastToString(sel);
	} else {
		*ok = false;
		head = xplCreateErrorNode(parent, BAD_CAST "select XPath expression '%s' evaluated to undef", select);
	}
	xmlXPathFreeObject(sel);
	return head;
}

xplSessionPtr xplDocSessionGetOrCreate(xplDocumentPtr doc, bool local)
{
	if (local)
	{
		if (!doc->local_session)
			doc->local_session = xplSessionCreateLocal();
		return doc->local_session;
	} else {
		if (!doc->shared_session)
		{
			if (!doc->parent)
				doc->shared_session = xplSessionCreateWithAutoId();
			else /* derived sessions are short-lived and don't need to be registered with session manager */
				doc->shared_session = xplSessionCreateLocal();
		}
		return doc->shared_session;
	}
}

bool xplDocSessionExists(xplDocumentPtr doc, bool local)
{
	if (local)
		return doc->local_session && xplSessionIsValid(doc->local_session);
	return doc->shared_session && xplSessionIsValid(doc->shared_session);
}

xmlChar* xplDocSessionGetId(xplDocumentPtr doc, bool local)
{
	xmlChar *id;
	xplSessionPtr session;

	session = local? doc->local_session: doc->shared_session;
	if (!session)
		return NULL;
	id = xplSessionGetId(session);
	return id? BAD_CAST XPL_STRDUP(id): NULL;
}

xmlNodePtr xplDocSessionGetObject(xplDocumentPtr doc, bool local, const xmlChar *name, const xmlNodePtr parent, const xmlChar *select, bool *ok)
{
	xplSessionPtr session;
	xmlNodePtr obj, ret;

	*ok = true;
	session = local? doc->local_session: doc->shared_session;
	if (!session)
		return NULL;
	if (local)
		obj = xplSessionGetObject(session, name);
	else
		obj = xplSessionAccessObject(session, name);
	if (select)
		ret = obj? _selectAndCopyNodes(doc, obj, select, parent, ok): NULL;
	else if (obj)
		ret = xplCloneNodeList(obj->children, parent, parent->doc);
	else
		ret = NULL;
	if (!local)
		xplSessionUnaccess(session);
	return ret;
}

xmlNodePtr xplDocSessionGetAllObjects(xplDocumentPtr doc, bool local, const xmlNodePtr parent, const xmlChar *select, bool *ok)
{
	xplSessionPtr session;
	xmlNodePtr obj, ret, cur;

	*ok = true;
	session = local? doc->local_session: doc->shared_session;
	if (!session)
		return NULL;
	if (local)
		obj = xplSessionGetAllObjects(session);
	else
		obj = xplSessionAccessAllObjects(session);
	if (select)
		ret = obj? _selectAndCopyNodes(doc, obj, select, parent, ok): NULL;
	else {
		cur = ret = obj? xplCloneNodeList(obj->children, parent, parent->doc): NULL;
		while (cur)
		{
			xplLiftNsDefs(parent, cur, cur->children);
			cur = cur->next;
		}
	}
	if (!local)
		xplSessionUnaccess(session);
	return ret;
}

bool xplDocSessionSetObject(xplDocumentPtr doc, bool local, const xmlNodePtr cur, const xmlChar *name)
{
	xplSessionPtr session;

	session = xplDocSessionGetOrCreate(doc, local);
	return xplSessionSetObject(session, cur, name);
}

void xplDocSessionRemoveObject(xplDocumentPtr doc, bool local, const xmlChar *name)
{
	xplSessionPtr session;

	session = local? doc->local_session: doc->shared_session;
	if (session)
		xplSessionRemoveObject(session, name);
}

void xplDocSessionClear(xplDocumentPtr doc, bool local)
{
	xplSessionPtr session;

	session = local? doc->local_session: doc->shared_session;
	if (session)
		xplSessionClear(session);
}

bool xplDocSessionGetSaMode(xplDocumentPtr doc)
{
	if (doc->force_sa_mode)
		return true;
	if (doc->local_session && xplSessionGetSaMode(doc->local_session))
		return true;
	if (doc->shared_session)
		return xplSessionGetSaMode(doc->shared_session);
	return false;
}

bool xplDocSessionSetSaMode(xplDocumentPtr doc, bool local, bool enable, xmlChar *password)
{
	xplSessionPtr session;

	session = xplDocSessionGetOrCreate(doc, local);
	return xplSessionSetSaMode(session, enable, password);
}

bool xplCheckNodeForXplNs(xplDocumentPtr doc, xmlNodePtr element)
{
	if (!element->ns)
		return false;
	return (element->ns == doc->root_xpl_ns) 
		|| (doc->root_xpl_ns && !xmlStrcmp(element->ns->href, doc->root_xpl_ns->href))
		|| !xmlStrcmp(element->ns->href, cfgXplNsUri);
}

/* content stuff for <xpl:content/> */
void getContentListInner(xplDocumentPtr doc, xmlNodePtr root, bool defContent, const xmlChar* id, xmlNodeSetPtr list);

static xmlNodeSetPtr getContentList(xplDocumentPtr doc, xmlNodePtr root, const xmlChar* id)
{
	/* ReszBuf not needed here: node sets grow by doubling */
	xmlNodeSetPtr nodeset = xmlXPathNodeSetCreate(NULL);
	if (!nodeset)
		return NULL;
	getContentListInner(doc, root, true, id, nodeset);
	return nodeset;
}

void getContentListInner(xplDocumentPtr doc, xmlNodePtr root, bool defContent, const xmlChar* id, xmlNodeSetPtr list)
{
	xmlChar* attr_id;
	xmlNodePtr c = root;

	while (c)
	{
		if (c->type != XML_ELEMENT_NODE) 
		{
			c = c->next;
			continue;
		}
		if (xplCheckNodeForXplNs(doc, c))
		{
			/* we're in the language namespace, let's make some more checks */
			if (!xmlStrcmp(c->name, BAD_CAST "content"))
			{
				if (!defContent)
				{
					attr_id = xmlGetNoNsProp(c, BAD_CAST "id");
					if (attr_id)
					{
						if (!xmlStrcmp(attr_id, id))
							xmlXPathNodeSetAddUnique(list, c);
						XPL_FREE(attr_id);
					}
				} else
					xmlXPathNodeSetAddUnique(list, c);
			} else if (!xmlStrcmp(c->name, BAD_CAST "define") /* these commands may have there own content */
				|| !xmlStrcmp(c->name, BAD_CAST "for-each")
				|| !xmlStrcmp(c->name, BAD_CAST "with")
				|| !xmlStrcmp(c->name, BAD_CAST "do")
			)
				getContentListInner(doc, c->children, false, id, list);
			else
				getContentListInner(doc, c->children, defContent, id, list);
		} else
			getContentListInner(doc, c->children, defContent, id, list);
		c = c->next;
	}
}

xmlNodePtr xplReplaceContentEntries(
	xplDocumentPtr doc,
	const xmlChar* id,
	xmlNodePtr oldElement,
	xmlNodePtr macroContent,
	xmlNodePtr parent
)
{
	xmlNodePtr ret, cur, new_content, tail, cloned;
	xmlNodeSetPtr content_cmds;
	xmlChar *select_attr;
	xmlChar *required_attr = NULL;
	bool required = false;
	xmlXPathObjectPtr sel;
	int i, j;
	xmlHashTablePtr content_cache = NULL;
	void *empty_content_marker = &empty_content_marker;

	ret = xplCloneNodeList(macroContent, parent, oldElement->doc);
	if (!ret)
		return NULL;
	content_cmds = getContentList(doc, ret, id);
	if (!content_cmds) /* out of memory */
	{
		xmlFreeNodeList(ret);
		return NULL;
	}
	if ((cfgMacroContentCachingThreshold >= 0)
		&& (content_cmds->nodeNr > 1)
		&& (cfgMacroContentCachingThreshold < content_cmds->nodeNr)
	)
		content_cache = xmlHashCreate(content_cmds->nodeNr);
    for (i = 0; i < content_cmds->nodeNr; i++)
	{
		cur = content_cmds->nodeTab[i];
		select_attr = xmlGetNoNsProp(cur, BAD_CAST "select");
		if (cfgWarnOnMissingMacroContent)
		{
			required = false;
			required_attr = xmlGetNoNsProp(cur, BAD_CAST "required");
			if (required_attr && !xmlStrcasecmp(required_attr, BAD_CAST "true"))
				required = true;
			XPL_FREE(required_attr);
		}
		if (select_attr)
		{
			if (content_cache) 
			{
				new_content = (xmlNodePtr) xmlHashLookup(content_cache, select_attr);
				if (new_content && (new_content != empty_content_marker))
					new_content = xplCloneNodeList(new_content, parent, oldElement->doc);
			} else
				new_content = NULL;
			if (!new_content) /* get new content by selector */ /* TODO _selectAndCopyNodes() */
			{
				sel = xplSelectNodesWithCtxt(doc->xpath_ctxt, oldElement, select_attr);
				if (sel)
				{
					if (sel->type == XPATH_NODESET)
					{
						if (sel->nodesetval)
						{
							tail = NULL;
							for (j = 0; j < sel->nodesetval->nodeNr; j++)
							{	
								cloned = xplCloneAsNodeChild(sel->nodesetval->nodeTab[j], parent);
								if (tail)
									tail->next = cloned;
								cloned->prev = tail;
								tail = cloned;
								if (!new_content)
									new_content = cloned;
							}
						}
					} else if (sel->type != XPATH_UNDEFINED) {
						new_content = xmlNewDocText(cur->doc, NULL);
						new_content->content = xmlXPathCastToString(sel);
					}
					xmlXPathFreeObject(sel);
				} else  /* if sel */
					new_content = xplCreateErrorNode(cur, BAD_CAST "invalid select XPath expression \"%s\"", select_attr);
				if (content_cache)
					xmlHashAddEntry(content_cache, select_attr, new_content? new_content: empty_content_marker);
			}
			if (new_content == empty_content_marker)
				new_content = NULL;
			if (required && !new_content)
				xplDisplayWarning(oldElement, BAD_CAST "missing macro content '%s'", select_attr);
			XPL_FREE(select_attr);
		} else { /* if (select_attr) */
			new_content = xplCloneNodeList(oldElement->children, cur->parent, oldElement->doc);
			if (required && !new_content)
				xplDisplayWarning(oldElement, BAD_CAST "missing macro content");
		}
		if (ret == cur)
			ret = new_content; /* <xpl:define name="A"><xpl:content/></xpl:define> */
		/* don't insert right now: we'll break caching */
		cur->_private = new_content;
    }
	for (i = 0; i < content_cmds->nodeNr; i++) /* actual replace */
	{
		cur = content_cmds->nodeTab[i];
		xplReplaceWithList(cur, (xmlNodePtr) cur->_private);
		xplDocDeferNodeDeletion(doc, cur);
	}
	if (content_cache)
		xmlHashFree(content_cache, NULL);
	xmlXPathFreeNodeSet(content_cmds);
	return ret;
}

/* node mode-based processing */
static void _xplExecuteMacro(xplDocumentPtr doc, xmlNodePtr element, xplMacroPtr macro, xplResultPtr result);
static void _xplExecuteCommand(xplDocumentPtr doc, xmlNodePtr element, xplResultPtr result);

void xplNodeApply(xplDocumentPtr doc, xmlNodePtr element, xplResultPtr result)
{
	xplMacroPtr macro;
	xmlHashTablePtr macros;
	
    if (doc->expand && (macro = xplMacroLookupByElement(element->parent, element)))
	{
		macro->times_encountered++;
        if (!macro->disabled_spin)
        {
			_xplExecuteMacro(doc, element, macro, result);     
			goto done;
        }
	}
    if (xplCheckNodeForXplNs(doc, element))
        _xplExecuteCommand(doc, element, result);
    else
    	xplNodeListApply(doc, element->children, result);
done:
	if ((macros = (xmlHashTablePtr) element->_private))
	{
		xplMacroTableFree(macros);
		element->_private = NULL;
	}
}

void xplNodeListApply(xplDocumentPtr doc, xmlNodePtr children, xplResultPtr result)
{
	xmlNodePtr tail, c = children;

	while (c)
	{
		if (c->type == XML_ELEMENT_NODE)
		{
			xplNodeApply(doc, c, result);
			if (c->parent && (c->parent->type & XPL_NODE_DELETION_MASK))
			{
				/* parent removed from inside, immediate stop */
				c->type = (xmlElementType) ((int) c->type & ~XPL_NODE_DELETION_MASK);
				if (result->has_list)
					xmlFreeNodeList(result->list); /* won't be used anyway */
				break;
			}
			tail = c->next;
			if (c->type & XPL_NODE_DELETION_MASK)
			{
				if (!(c->type & XPL_NODE_DELETION_DEFERRED_FLAG))
				{
					c->type = (xmlElementType) ((int) c->type & ~XPL_NODE_DELETION_REQUEST_FLAG);
					xmlUnlinkNode(c);
					xmlFreeNode(c); /* with children */
				} else
					NOOP(); /* if DELETION_DEFERRED is set contents will be deleted after processing */
			} else if (result->has_list) {
				if (result->list && result->repeat)
					tail = result->list;
				if (c->nsDef)
					xplLiftNsDefs(c->parent, c, result->list);
				xplReplaceWithList(c, result->list);
				/* e.g. :delete removing itself */
				c->type = (xmlElementType) ((int) c->type & ~XPL_NODE_DELETION_MASK);
				xplDocDeferNodeDeletion(doc, c);
			} /* otherwise do nothing */
		} else if (c->type == XML_TEXT_NODE) {
			tail = c->next;
			if (!xstrStrNonblank(c->content) && !doc->indent_spin) /* remove formatting unless instructed otherwise */
			{
				xmlUnlinkNode(c);
				xmlFreeNode(c);
			}
		} else if (c->type == XML_COMMENT_NODE) {
			/* remove comments */
			tail = c->next;
			xmlUnlinkNode(c);
			xmlFreeNode(c);
		} else 
			/* DTD, PI etc - copy to output */
			tail = c->next;
		c = tail;
	}
	ASSIGN_RESULT(NULL, false, false);
}

void _xplExecuteMacro(xplDocumentPtr doc, xmlNodePtr element, xplMacroPtr macro, xplResultPtr result)
{
	xmlNodePtr out, prev_caller;
	xplMacroPtr prev_macro;

	doc->macro_nesting_level++;
	if (doc->macro_nesting_level > cfgMaxRecursionDepth)
	{
		ASSIGN_RESULT(xplCreateErrorNode(element, BAD_CAST "recursion depth exhausted (infinite loop in macro?..)"), true, true);
		return;
	}
	prev_macro = doc->current_macro;
	prev_caller = macro->caller;
	macro->caller = element;
	doc->current_macro = macro;
	macro->times_called++;
	if (element->children && cfgWarnOnExpandedMacroContent && (macro->expansion_state == XPL_MACRO_EXPANDED))
		xplDisplayWarning(element, BAD_CAST "child nodes present in expanded macro caller");
	switch (macro->expansion_state)
	{
	case XPL_MACRO_EXPAND_ALWAYS:
	case XPL_MACRO_EXPAND_ONCE:
		/* :inherit may be called, let's keep contents. not clearing parent intentionally */
		macro->node_original_content = element->children; 
		out = xplReplaceContentEntries(doc, macro->id, element, macro->content, element->parent);
		xplSetChildren(element, out);
		xplNodeListApply(doc, element->children, result);
		out = xplDetachChildren(element);
		if (!out) /* contents could be removed by :return */
			out = macro->return_value;
		break;
	case XPL_MACRO_EXPANDED:
		out = xplCloneNodeList(macro->content, element->parent, element->doc);
		break;
	default:
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		out = xmlNewDocText(doc->document, BAD_CAST "internal error");
	}
	if (macro->expansion_state == XPL_MACRO_EXPAND_ONCE)
	{
		xmlFreeNodeList(macro->content);
		macro->content = xplCloneNodeList(out, element->parent, element->doc);
		macro->expansion_state = XPL_MACRO_EXPANDED;
	}
	ASSIGN_RESULT(out, false, true);
	doc->macro_nesting_level--;
	doc->current_macro = prev_macro;
	macro->caller = prev_caller;
	macro->return_value = NULL; 
	if (macro->node_original_content)
	{
		xplDocDeferNodeListDeletion(doc, macro->node_original_content);
		macro->node_original_content = NULL;
	}
}

void _xplExecuteCommand(xplDocumentPtr doc, xmlNodePtr element, xplResultPtr result)
{
	xmlNodePtr error;
	xplCommandPtr cmd;
	xplCommandInfo cmd_info;

	cmd = xplGetCommand(element);
	if (!cmd)
	{
		if (cfgWarnOnUnknownCommand && doc->expand)
			xplDisplayWarning(element, BAD_CAST "unknown command");
		xplNodeListApply(doc, element->children, result);
		return;
	}
	if (!doc->expand && !(cmd->flags & XPL_CMD_FLAG_ALWAYS_EXPAND))
	{
		xplNodeListApply(doc, element->children, result);
		return;
	}
	memset(&cmd_info, 0, sizeof(cmd_info));
	cmd_info.element = element;
	cmd_info.document = doc;
	cmd_info.xpath_ctxt = doc->xpath_ctxt;
	if (!(error = xplFillCommandInfo(cmd, &cmd_info, true)))
	{
		if (cmd->prologue)
			cmd->prologue(&cmd_info);
		/* element could be removed (:with) */
		if (!(element->type & XPL_NODE_DELETION_MASK))
			xplNodeListApply(doc, cmd_info.element->children, result);
		/* element could be removed by its children */
		if (!(element->type & XPL_NODE_DELETION_MASK))
		{
			if (!(error = xplFillCommandInfo(cmd, &cmd_info, false)))
				cmd->epilogue(&cmd_info, result);
			else
				ASSIGN_RESULT(error, true, true);
		}
		if (cmd->restore_state)
			cmd->restore_state(&cmd_info);
	} else
		ASSIGN_RESULT(error, true, true);
	xplClearCommandInfo(cmd, &cmd_info);
}

xplMacroPtr xplAddMacro(
	xplDocumentPtr doc, 
	xmlNodePtr macro,
	xplQName qname,
	xmlNodePtr destination, 
	xplMacroExpansionState expansionState,
	bool replace,
	xmlChar *id
)
{
	xmlHashTablePtr macros; 
	xplMacroPtr mb, prev_def = NULL, prev_macro;
	xplResult tmp_result;
	xmlNsPtr ns;

	if (!replace || (replace && cfgWarnOnMacroRedefinition))
		prev_def = xplMacroLookupByQName(macro->parent, qname);
	if (replace && cfgWarnOnMacroRedefinition && prev_def)
		xplDisplayWarning(macro, BAD_CAST "macro '%s%s%s' redefined, previous line: %d",
			qname.ns && qname.ns->href? qname.ns->href: BAD_CAST "",
			qname.ns && qname.ns->href? ":": "",
			qname.ncname, prev_def->line);
	if (!replace && prev_def)
		return NULL;
	mb = xplMacroCreate(id, NULL, expansionState);
	if (expansionState == XPL_MACRO_EXPANDED)
	{
		prev_macro = doc->current_macro;
		doc->current_macro = mb;
		mb->caller = macro;
		xplNodeListApply(doc, macro->children, &tmp_result);
		doc->current_macro = prev_macro;
	}
	mb->content = xplDetachChildren(macro);
	if (!mb->content)
	{
		mb->content = mb->return_value;
		mb->return_value = NULL;
	}
	mb->ns_defs = macro->nsDef;
	macro->nsDef = NULL;
	macros = (xmlHashTablePtr) destination->_private;
	if (!macros)
	{
		macros = xmlHashCreate(cfgInitialMacroTableSize);
		destination->_private = macros;
	}
	mb->qname.ns = qname.ns;
	mb->qname.ncname = BAD_CAST XPL_STRDUP((char*) qname.ncname);
	ns = macro->nsDef;
	while (ns)
	{
		if (mb->qname.ns == ns)
		{
			mb->ns_is_duplicated = true;
			mb->qname.ns = xmlCopyNamespace(ns);
		}
		ns = ns->next;
	}
	mb->line = macro->line;
	mb->parent = destination;
	xplMacroAddToHash(macros, mb);
	return mb;
}

/* End of low-level doc traverse */

static void xplCheckRootNs(xplDocumentPtr doc, xmlNodePtr root)
{
	xmlNsPtr ns;

	ns = root->nsDef;
	while (ns)
	{
		if (!xmlStrcmp(ns->href, cfgXplNsUri))
		{
			doc->root_xpl_ns = ns;
			return;
		}
		ns = ns->next;
	}
	ns = root->nsDef;
	while (ns)
	{
		if (!xmlStrcmp(ns->prefix, BAD_CAST "xpl"))
		{
			doc->root_xpl_ns = ns;
			break;
		}
		ns = ns->next;
	}
	if (cfgWarnOnInvalidXplNsUri)
	{
		if (doc->root_xpl_ns)
			xplDisplayMessage(XPL_MSG_WARNING, BAD_CAST "XPL namespace uri \"%s\" differs from configured \"%s\", document \"%s\"",
			doc->root_xpl_ns->href, cfgXplNsUri, doc->document->URL);
		else
			xplDisplayMessage(XPL_MSG_WARNING, BAD_CAST "Cannot find XPL namespace on the root document element, document \"%s\"", doc->document->URL);
	}
}

xplError xplDocumentApply(xplDocumentPtr doc)
{
	xmlNodePtr root_element;
	xplResult res;
	xmlDocPtr error_doc;
	xmlNodePtr error_node;
	xplError ret = XPL_ERR_INVALID_DOCUMENT;

	if (!doc)
		return XPL_ERR_INVALID_INPUT;
	if (!parser_loaded)
		return XPL_ERR_NO_PARSER;
	if (doc->document)
	{
		if ((root_element = xplFirstElementNode(doc->document->children)))
		{
			if (!doc->parent || doc->async)
			{
				/* don't change this to a call to xprInterlockedIncrement(). global_conf_mutex must be acquired here. */
				SUCCEED_OR_DIE(xprMutexAcquire(&global_conf_mutex));
				active_thread_count++;
				SUCCEED_OR_DIE(xprMutexRelease(&global_conf_mutex));
			}
			xplCheckRootNs(doc, root_element);
			xplNodeApply(doc, root_element, &res);
#ifdef _THREADING_SUPPORT
			xplFinalizeDocThreads(doc, false);
#endif
			if (doc->fatal_content) /* xpl:fatal called */
			{
				root_element->type &= ~XPL_NODE_DELETION_MASK;
				xmlFreeNodeList(doc->document->children); /* lists don't overlap, DDN calls xmlUnlinkNode() */
				doc->document->children = doc->document->last = doc->fatal_content;
				doc->fatal_content->parent = (xmlNodePtr) doc->document;
				ret = doc->status = XPL_ERR_FATAL_CALLED;
			} else
				ret = doc->status = XPL_ERR_NO_ERROR;
			if (!doc->parent || doc->async)
				xprInterlockedDecrement(&active_thread_count);
		} /* if (root_element) */
	} else {
		error_doc = xmlNewDoc(BAD_CAST "1.0");
		error_node = xmlNewDocNode(error_doc, NULL, ERROR_NODE_NAME, doc->error);
		error_doc->children = error_doc->last = error_node;
		error_node->parent = (xmlNodePtr) error_doc;
		doc->document = error_doc;
		ret = doc->status = XPL_ERR_INVALID_DOCUMENT;
	}
	if (cfgDebugSaveFile && !doc->parent) /* saving derived documents makes no sense */
	{
		if (!xplSaveXmlDocToFile(doc->document, cfgDebugSaveFile, (char*) cfgDefaultEncoding, XML_SAVE_FORMAT))
			xplDisplayMessage(XPL_MSG_WARNING, BAD_CAST "Cannot save debug output to \"%s\", check that the file location exists and is writable", cfgDebugSaveFile);
	}
	return ret;
}

void xplLockThreads(bool doLock)
{
	if (doLock)
	{
		xprInterlockedDecrement(&active_thread_count);
		SUCCEED_OR_DIE(xprMutexAcquire(&global_conf_mutex));
		while (active_thread_count)
			xprSleep(100);
	} else {
		xprInterlockedIncrement(&active_thread_count);
		SUCCEED_OR_DIE(xprMutexRelease(&global_conf_mutex));
	}
}

xplGetAppTypeFunc xplSetGetAppTypeFunc(xplGetAppTypeFunc f)
{
	xplGetAppTypeFunc tmp = get_app_type_func;
	get_app_type_func = f;
	return tmp;
}

xmlChar* xplGetAppType()
{
	if (get_app_type_func)
		return get_app_type_func();
	return BAD_CAST "unknown";
}

static bool _ssInitCore()
{
	if (!xprMutexInit(&global_conf_mutex))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return false;
	}
	return true;
}

static void _ssStopCore()
{
	if (!xprMutexCleanup(&global_conf_mutex))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
}

typedef struct _ParserStartStopStep
{
	bool (*start_fn)(void);
	void (*stop_fn)(void);
	char *name;
} ParserStartStopStep, *ParserStartStopStepPtr;

static ParserStartStopStep start_stop_steps[] =
{
	{ xplInitOptions, xplCleanupOptions, "options" },
	{ xplInitNamePointers, NULL, "name pointers" },
	{ xplInitLibraryVersions, xplCleanupLibraryVersions, "library versions" },
	{ xplInitWrapperMap, xplCleanupWrapperMap, "wrappers" },
	{ xplInitDatabases, xplCleanupDatabases, "databases" },
	{ xplInitMessages, xplCleanupMessages, "messages" },
	{ xplInitCommands, xplCleanupCommands, "commands" },
	{ xplLoadableModulesInit, xplLoadableModulesCleanup, "loadable modules" },
	{ xplRegisterBuiltinCommands, xplUnregisterBuiltinCommands, "builtin commands" },
	{ xplSessionManagerInit, xplSessionManagerCleanup, "session" },
	{ _ssInitCore, _ssStopCore, "core" },
};
#define START_STOP_STEP_COUNT (sizeof(start_stop_steps) / sizeof(start_stop_steps[0]))

xplError xplInitParser(bool verbose)
{
	int i, j;

	if (parser_loaded)
		return XPL_ERR_NO_ERROR;
	for (i = 0; i < (int) START_STOP_STEP_COUNT; i++)
	{
		if (!start_stop_steps[i].start_fn)
			continue;
		if (verbose)
			printf("Initializing: %s... ", start_stop_steps[i].name);
		if (!start_stop_steps[i].start_fn())
		{
			if (verbose)
				printf("FAILED!\n");
			else
				printf("Failed to initialize %s!\n", start_stop_steps[i].name);
			for (j = i - 1; j >= 0; j--)
			{
				if (!start_stop_steps[j].stop_fn)
					continue;
				start_stop_steps[j].stop_fn();
			}
			return XPL_ERR_NO_PARSER;
		} else if (verbose)
			printf("OK\n");
	}
	parser_loaded = true;
	return XPL_ERR_NO_ERROR;
}

void xplDoneParser()
{
	int i;

	if (!parser_loaded)
		return;
	for (i = START_STOP_STEP_COUNT - 1; i >= 0; i--)
	{
		if (!start_stop_steps[i].stop_fn)
			continue;
		start_stop_steps[i].stop_fn();
	}
	parser_loaded = false;
}

bool xplParserLoaded()
{
	return parser_loaded;
}

xmlChar* xplGetDocRoot()
{
	return cfgDocRoot;
}

void xplSetDocRoot(xmlChar *new_root)
{
	if (cfgDocRoot)
		XPL_FREE(cfgDocRoot);
	/* don't remove strdup() here unless you want to spent the rest of your evening debugging code */
	cfgDocRoot = BAD_CAST XPL_STRDUP((char*) new_root);
}

xmlChar* xplFullFilename(const xmlChar* file, const xmlChar* aDocRoot)
{
	xmlChar* ret;
	int len;

	if (file && xmlStrlen(file) && ((*file == '/') || (*file == '\\'))) /* from FS root */
	{
		aDocRoot = xplGetDocRoot();
	}
	len = xmlStrlen(aDocRoot) + xmlStrlen(file) + 1;
	ret = (xmlChar*) XPL_MALLOC(len);
	*ret = 0;
	if (aDocRoot)
		strcpy((char*) ret, (const char*) aDocRoot);
	if (file)
		strcat((char*) ret, (const char*) file);
	return ret;
}

#ifdef _DEBUG_COALESCING
void checkCoalescing(xmlNodePtr cur)
{
	while (cur)
	{
		if ((cur->type == XML_TEXT_NODE) && cur->next && (cur->next->type == XML_TEXT_NODE))
		{
			/* oops: two text nodes in row */
			printf("Coalescing error: cur \"%s\", next \"%s\"\n", cur->content, cur->next->content);
		}
		checkCoalescing(cur->children);
		cur = cur->next;
	}
}
#define CHECK_COALESCING(x) checkCoalescing(x)
#else
#define CHECK_COALESCING(x) ((void)0)
#endif

xplError xplProcessFile(xmlChar *aDocRoot, xmlChar *aFilename, xplParamsPtr aParams, xplSessionPtr aSession, xplDocumentPtr *docOut)
{
	xplError ret;

	*docOut = xplDocumentCreateFromFile(aDocRoot, aFilename, aParams, aSession);
	if (!*docOut)
		return XPL_ERR_DOC_NOT_CREATED;
	(*docOut)->status = XPL_ERR_NOT_YET_REACHED;
	ret = xplDocumentApply(*docOut);
	if (ret >= XPL_ERR_NO_ERROR)
		CHECK_COALESCING((*docOut)->document->children);
	return ret;
}

xplError xplProcessStartupFile(xmlChar *aDocRoot, xmlChar *aFilename)
{
	xplParamsPtr params = NULL;
	xplSessionPtr session = NULL;
	xplDocumentPtr doc = NULL;
	xplError ret = XPL_ERR_DOC_NOT_CREATED;

	if (!(params = xplParamsCreate()))
		goto done;
	if (!(session = xplSessionCreateLocal()))
		goto done;
	if (!(doc = xplDocumentCreateFromFile(aDocRoot, aFilename, params, session)))
		goto done;
	doc->force_sa_mode = true;
	doc->status = XPL_ERR_NOT_YET_REACHED;
	ret = xplDocumentApply(doc);
	if (ret >= XPL_ERR_NO_ERROR)
		CHECK_COALESCING(doc->document->children);
done:
	if (doc)
		xplDocumentFree(doc);
	if (params)
		xplParamsFree(params);
	if (session)
		xplSessionFreeLocal(session);
	return ret;
}

xplError xplProcessFileEx(xmlChar *basePath, xmlChar *relativePath, xplParamsPtr aParams, xplSessionPtr aSession, xplDocumentPtr *docOut)
{
	xmlChar *norm_path;
	xmlChar *norm_filename;
	xplError status;

	xstrComposeAndSplitPath(basePath, relativePath, &norm_path, &norm_filename);
	status = xplProcessFile(norm_path, norm_filename, aParams, aSession, docOut);
	if (norm_path)
		XPL_FREE(norm_path);
	return status;
}
