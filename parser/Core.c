#include "Configuration.h"
#include "Core.h"
#include "Messages.h"
#include "Options.h"
#include "Register.h"
#include "Utils.h"
#include "abstraction/ExtFeatures.h"

/* TODO */
//#include <locstdint.h>
/* _***printf() */
#include <stdio.h> 

/* internal statics */
static bool parser_loaded = 0;
static xmlChar *config_file_path = NULL;
XPR_SEMAPHORE *global_thread_semaphore = NULL;
xplLockThreadsFunc lock_threads_func = NULL;
xplGetAppTypeFunc get_app_type_func = NULL;

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
	case XPL_ERR_NO_CONFIG_FILE:
		return BAD_CAST "config file couldn't be read";
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
	case XPL_ERR_NO_CONFIG_FILE:
		return BAD_CAST "invalid_config";
	case XPL_ERR_NOT_YET_REACHED:
		return BAD_CAST "not_reached";
	default:
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return BAD_CAST "unknown";
	}
}

/* per-document node stack (for xpl:stack-xxx) */
void xplPushToDocStack(xplDocumentPtr doc, xmlNodePtr node)
{
	xmlNodePtr new_parent;

	if (!doc)
		return;
	new_parent = xmlNewDocNode(doc->document, NULL, BAD_CAST "carrier", NULL);
	setChildren(new_parent, node->children);
	setChildren(node, new_parent);
	makeNsIndepTree(new_parent);
	setChildren(node, NULL);
	new_parent->prev = doc->stack;
	doc->stack = new_parent;
}

xmlNodePtr xplPopFromDocStack(xplDocumentPtr doc, xmlNodePtr parent)
{
	xmlNodePtr ret, tmp;

	if (!doc || !doc->stack)
		return NULL;
	/* Мы вынуждены использовать клонирование, т.к. могут возникнуть проблемы с ns */
	ret = cloneNodeList(doc->stack->children, parent, parent->doc);
	tmp = doc->stack;
	doc->stack = tmp->prev;
	xmlUnlinkNode(tmp);
	xmlFreeNode(tmp);
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

/* Command routines */
static xmlXPathObjectPtr xplSelectNodesInner(xmlXPathContextPtr ctxt, xmlNodePtr src, xmlChar *expr)
{
    xmlXPathObjectPtr ret;
    
	if (!ctxt)
		return NULL;
    ctxt->namespaces = xmlGetNsList(src->doc, src);
    ctxt->nsNr = 0;
    if (ctxt->namespaces) 
	{
		while (ctxt->namespaces[ctxt->nsNr])
            ctxt->nsNr++;
    }
	ctxt->node = src;
	ctxt->doc = src->doc;
	ret = xmlXPathEvalExpression(expr, ctxt);
    if (ctxt->namespaces)
		xmlFree(ctxt->namespaces);
    return ret;
}

xmlXPathObjectPtr xplSelectNodes(xplDocumentPtr doc, xmlNodePtr src, xmlChar *expr)
{
	xmlXPathContextPtr ctxt;

	if (!src | !doc)
		return NULL;
	ctxt = doc->xpath_ctxt;
	return xplSelectNodesInner(ctxt, src, expr);
}

void xplDeferNodeDeletion(ReszBufPtr buf, xmlNodePtr cur)
{
	if (!buf | !cur)
		return;
	if ((int) cur->type & XML_NODE_DELETION_DEFERRED_FLAG)
		return;
	markDOSAxisForDeletion(cur, XML_NODE_DELETION_DEFERRED_FLAG, true);
	rbAddDataToBuf(buf, &cur, sizeof(xmlNodePtr));
}

void xplDeferNodeListDeletion(ReszBufPtr buf, xmlNodePtr cur)
{
	if (!buf)
		return;
	while (cur)
	{
		xplDeferNodeDeletion(buf, cur);
		cur = cur->next;
	}
}

void xplDocDeferNodeDeletion(xplDocumentPtr doc, xmlNodePtr cur)
{
	if (!doc)
		return;
	if (cfgFoolproofDestructiveCommands && doc->iterator_spinlock)
		xplDeferNodeDeletion(doc->deleted_nodes, cur);
	else {
		xmlUnlinkNode(cur);
		xmlFreeNode(cur);
	}
}

void xplDocDeferNodeListDeletion(xplDocumentPtr doc, xmlNodePtr cur)
{
	if (!doc)
		return;
	if (cfgFoolproofDestructiveCommands && doc->iterator_spinlock)
		xplDeferNodeListDeletion(doc->deleted_nodes, cur);
	else
		xmlFreeNodeList(cur);
}

void xplDeleteDeferredNodes(ReszBufPtr buf)
{
	size_t i = 0;
	xmlNodePtr *p = rbGetBufContent(buf);
	for (i = 0; i < rbGetBufContentSize(buf) / sizeof(xmlNodePtr); i++, p++)
	{
		markDOSAxisForDeletion(*p, XML_NODE_DELETION_MASK, false);
		xmlUnlinkNode(*p);
		xmlFreeNode(*p);
	}
	rbRewindBuf(buf);
}

XPR_SEMAPHORE *xplGetGlobalThreadSemaphore(void)
{
	return global_thread_semaphore;
}

XPR_SEMAPHORE *xplSetGlobalThreadSemaphore(XPR_SEMAPHORE *p)
{
	XPR_SEMAPHORE *tmp = global_thread_semaphore;
	global_thread_semaphore = p;
	return tmp;
}

xplLockThreadsFunc xplSetLockThreadsFunc(xplLockThreadsFunc f)
{
	xplLockThreadsFunc tmp = lock_threads_func;
	lock_threads_func = f;
	return tmp;
}

void xplLockThreads(bool doLock)
{
	if (lock_threads_func)
		lock_threads_func(doLock);
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

bool xplGetDocByRole(xplDocumentPtr docIn, xmlChar *strRole, xplDocumentPtr *docOut)
{
	xplDocRole role;

	if (!strRole)
	{
		*docOut = docIn;
		return true;
	}
	if (!docIn)
	{
		*docOut = NULL;
		return true;
	}
	if ((role = xplDocRoleFromString(strRole)) == XPL_DOC_ROLE_UNKNOWN)
		return false;
	switch (role)
	{
	case XPL_DOC_ROLE_PROLOGUE: 
		*docOut = docIn->prologue;
		return true;
	case XPL_DOC_ROLE_MAIN:
		*docOut = docIn->main;
		return true;
	case XPL_DOC_ROLE_EPILOGUE:
		*docOut = docIn->epilogue;
		return true;
	default:
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	}
	*docOut = NULL;
	return false;
}

/* common document initialization part */
xplDocumentPtr xplDocumentInit(xmlChar *aAppPath, xplParamsPtr aEnvironment, xplSessionPtr aSession)
{
	size_t path_len;
	xplDocumentPtr doc;
	xmlChar delim;
	
	doc = (xplDocumentPtr) xmlMalloc(sizeof(xplDocument));
	if (!doc)
		return NULL;
	memset(doc, 0, sizeof(xplDocument));

	if (!aAppPath)
		aAppPath = cfgDocRoot;
	if (aAppPath && *aAppPath)
	{
		path_len = xmlStrlen(aAppPath);
		delim = 0;
		if (aAppPath[path_len - 1] == XPR_PATH_INVERSE_DELIM)
			delim = XPR_PATH_DELIM;
		else if (aAppPath[path_len - 1] != XPR_PATH_DELIM)
			path_len++;
		doc->app_path = (xmlChar*) xmlMalloc(path_len + 1);
		if (!doc->app_path) 
		{
			xplDocumentFree(doc);
			return NULL;
		}
		strcpy((char*) doc->app_path, (const char*) aAppPath);
		if (delim)
			doc->app_path[path_len - 1] = delim;
		else if (doc->app_path[path_len - 1] != XPR_PATH_DELIM) {
			doc->app_path[path_len - 1] = XPR_PATH_DELIM;
			doc->app_path[path_len] = 0;
		}
	} else {
		doc->app_path = NULL;
	}
	doc->xpath_ctxt = xmlXPathNewContext(NULL);
	if (!doc->xpath_ctxt)
	{
		xplDocumentFree(doc);
		return NULL;
	}
	doc->deleted_nodes = rbCreateBufParams(32, RESZ_BUF_GROW_DOUBLE, 2);
	if (!doc->deleted_nodes)
	{
		xplDocumentFree(doc);
		return NULL;
	}
	doc->session = aSession;
	doc->environment = aEnvironment;
	doc->status = XPL_ERR_NOT_YET_REACHED;
	return doc;
}

xplDocumentPtr xplDocumentCreateFromFile(xmlChar *aAppPath, xmlChar *aFilename, xplParamsPtr aEnvironment, xplSessionPtr aSession)
{
	xplDocumentPtr ret;	
	size_t path_len;
	size_t max_fn_len;

	if (!aFilename)
		return NULL;
	ret = xplDocumentInit(aAppPath, aEnvironment, aSession);
	path_len = xmlStrlen(ret->app_path);
	max_fn_len = path_len + xmlStrlen(aFilename) + 1;
	ret->filename = (xmlChar*) xmlMalloc(max_fn_len);
	if (!ret->filename)
	{
		xplDocumentFree(ret);
		return NULL;
	}
	if (ret->app_path)
		strcpy((char*) ret->filename, (const char*) ret->app_path);
	strcpy((char*) ret->filename + path_len, (const char*) aFilename);

	ret->document = xmlParseFile((const char*) ret->filename);
	if (ret->document)
	{
		ret->error = NULL;
	} else {
		ret->error = getLastLibxmlError();
		xmlResetLastError();
	}

	return ret;
}

xplDocumentPtr xplDocumentCreateFromMemory(xmlChar* aAppPath, xmlChar *aOrigin,  xplParamsPtr aEnvironment, xplSessionPtr aSession, xmlChar *encoding)
{
	xplDocumentPtr ret = xplDocumentInit(aAppPath, aEnvironment, aSession);

	ret->origin = aOrigin;
	ret->document = xmlReadMemory((const char*) aOrigin, xmlStrlen(aOrigin), NULL, (const char*) encoding, 0);
	if (ret->document)
	{
		ret->error = NULL;
	} else {
		ret->error = getLastLibxmlError();
		xmlResetLastError();
	}

	return ret;
}
void xplDocumentFree(xplDocumentPtr doc)
{
	if (!doc)
		return;
#ifdef _THREADING_SUPPORT
	if (doc->threads)
	{
		/* Перестрахуемся */
		doc->discard_suspended_threads = true;
		xplStartDelayedThreads(doc);
		xplWaitForChildThreads(doc);
		rbFreeBuf(doc->threads);
		if (!xprMutexCleanup(&doc->thread_landing_lock))
			DISPLAY_INTERNAL_ERROR_MESSAGE();
	}
#endif
	if (doc->app_path) xmlFree(doc->app_path);
	if (doc->filename) xmlFree(doc->filename);
	if (doc->error) xmlFree(doc->error);
	if (doc->xpath_ctxt) xmlXPathFreeContext(doc->xpath_ctxt);
	if (doc->response) xmlFree(doc->response);
	xplClearDocStack(doc);
	/* ещё раз перестрахуемся */
	xplDeleteDeferredNodes(doc->deleted_nodes);
	rbFreeBuf(doc->deleted_nodes);
	/* Освободим документ последним */
	if (doc->document) xmlFreeDoc(doc->document);
	xmlFree(doc);
}

#ifdef _THREADING_SUPPORT
void xplEnsureDocThreadSupport(xplDocumentPtr doc)
{
	if (!global_thread_semaphore)
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return false;
	}
	if (doc->threads)
		return;
	if (!xprMutexInit(&doc->thread_landing_lock))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return false;
	}
	doc->threads = rbCreateBufParams(2*sizeof(XPR_THREAD_HANDLE), RESZ_BUF_GROW_INCREMENT, 2*sizeof(XPR_THREAD_HANDLE));
	if (!doc_threads)
	{
		if (!xprMutexCleanup(&doc->thread_landing_lock))
			DISPLAY_INTERNAL_ERROR_MESSAGE();
		return false;
	}
	return true;
}

void xplWaitForChildThreads(xplDocumentPtr doc)
{
	XPR_THREAD_HANDLE *handles;
	size_t count;

	if (!doc->threads)
		return;
	handles = (XPR_THREAD_HANDLE*) rbGetBufContent(doc->threads);
	count = rbGetBufContentSize(doc->threads) / sizeof(XPR_THREAD_HANDLE);
	if (count)
		xprWaitForThreads(handles, (int) count);
	rbRewindBuf(doc->threads);
}

/* Thread wrapper */
XPR_THREAD_ROUTINE_RESULT XPR_THREAD_ROUTINE_CALL xplDocThreadWrapper(XPR_THREAD_ROUTINE_PARAM p)
{
	xplDocumentPtr doc = (xplDocumentPtr) p;
	xmlNodePtr content;
	xplError err;

	if (!doc)
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE;
		XPR_EXIT_THREAD(-1);
	}
	if (!(doc->parent->discard_suspended_threads && doc->thread_was_suspended))
	{
		if (!xprSemaphoreAcquire(global_thread_semaphore))
		{
			DISPLAY_INTERNAL_ERROR_MESSAGE();
			/* don't unlink/free the landing point here - possible race condition */
			goto done;
		}
		err = xplDocumentApply(doc);
		if ((err != XPL_ERR_NO_ERROR) && (err != XPL_ERR_FATAL_CALLED))
			/* Такого не должно быть, но… */
			content = xplCreateErrorNode(doc->landing_point, BAD_CAST "error processing child document: \"%s\"", xplErrorToString(err));
		else 
			/* Корневой элемент фиктивен */
			content = detachContent((xmlNodePtr) doc->document->children);
		XPR_ACQUIRE_LOCK(doc->parent->thread_landing_lock);
		xmlSetListDoc(content, doc->parent->document);
		xplDocDeferNodeDeletion(doc, replaceWithList(doc->landing_point, content));
		if (!xprMutexRelease(&doc->parent->thread_landing_lock))
			DISPLAY_INTERNAL_ERROR_MESSAGE();
		if (!xprSemaphoreRelease(global_thread_semaphore))
			DISPLAY_INTERNAL_ERROR_MESSAGE();
	} else {
		xmlUnlinkNode(doc->landing_point);
		xmlFreeNode(doc->landing_point);
	}
done:
	xplParamsFree(doc->environment);
	doc->document->intSubset = NULL;
	xplDocumentFree(doc);
	XPR_EXIT_THREAD(0);
}

bool xplStartChildThread(xplDocumentPtr doc, xplDocumentPtr child, bool immediateStart)
{
	XPR_THREAD_HANDLE handle;
	if (!doc || !child) 
		return false;
	if (!xplEnsureDocThreadSupport(doc))
		return false;
	XPR_START_THREAD_SUSPENDED(handle, xplDocThreadWrapper, (XPR_THREAD_ROUTINE_PARAM) child);
	if (!handle)
		return false;
	rbAddDataToBuf(doc->threads, &handle, sizeof(XPR_THREAD_HANDLE));
	child->thread_was_suspended = !immediateStart;
	if (immediateStart)
		XPR_RESUME_THREAD(handle);
	else
		doc->has_suspended_threads = true;
	return true;
}

void xplStartDelayedThreads(xplDocumentPtr doc)
{
	size_t pool_size, i;
	XPR_THREAD_HANDLE *handles;
	if (!doc || !doc->threads || !doc->has_suspended_threads)
		return;
	pool_size = rbGetBufContentSize(doc->threads) / sizeof(XPR_THREAD_HANDLE);
	for (i = 0, handles = rbGetBufContent(doc->threads); i < pool_size; i++, handles++)
	{
		XPR_RESUME_THREAD(*handles);
	}
	doc->has_suspended_threads = false;
}
#endif


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

xmlNodeSetPtr getContentList(xplDocumentPtr doc, xmlNodePtr root, const xmlChar* id)
{
	/* на ReszBuf можно не переписывать, набор узлов и так растёт по удвоению */
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
			/* мы в пространстве имён языка, сделаем несколько дополнительных проверок */
			if (!xmlStrcmp(c->name, BAD_CAST "content"))
			{
				if (!defContent)
				{
					attr_id = xmlGetNoNsProp(c, BAD_CAST "id");
					if (attr_id)
					{
						if (!xmlStrcmp(attr_id, id))
							xmlXPathNodeSetAddUnique(list, c);
						xmlFree(attr_id);
					}
                } else
					xmlXPathNodeSetAddUnique(list, c);
            } else if (!xmlStrcmp(c->name, BAD_CAST "define") /* после попадания в команду, имеющую контент, убираем флаг "контент по умолчанию" */
				|| !xmlStrcmp(c->name, BAD_CAST "for-each")
				|| !xmlStrcmp(c->name, BAD_CAST "with")
				|| !xmlStrcmp(c->name, BAD_CAST "do")) {
				getContentListInner(doc, c->children, false, id, list);
			} else {
				getContentListInner(doc, c->children, defContent, id, list);
            }
		} else {
			getContentListInner(doc, c->children, defContent, id, list);
		}
		c = c->next;
	}
}

xmlNodePtr xplReplaceContentEntries(xplDocumentPtr doc, const xmlChar* id, xmlNodePtr oldElement, xmlNodePtr macroContent)
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

	ret = cloneNodeList(macroContent, oldElement->parent, oldElement->doc);
	if (!ret)
		return NULL;
	content_cmds = getContentList(doc, ret, id);
	if (!content_cmds) /* чтобы здесь не путаться: это не "нет команд content", это "нет памяти под хранилище" */
	{
		xmlFreeNodeList(ret);
		return NULL;
	}
	if ((cfgMacroContentCachingThreshold) >= 0 /* кэширование включено */
		&& (content_cmds->nodeNr > 1) /* нет смысла кэшировать одиночную выборку */
		&& (cfgMacroContentCachingThreshold < content_cmds->nodeNr) /* кэширование уместно */)
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
			xmlFree(required_attr);
		}
		if (select_attr)
		{
			if (content_cache) 
			{
				new_content = (xmlNodePtr) xmlHashLookup(content_cache, select_attr);
				if (new_content && (new_content != empty_content_marker))
					new_content = cloneNodeList(new_content, oldElement, oldElement->doc);
			} else
				new_content = NULL;
			if (!new_content) /* получим новое содержимое по селектору */
			{
				sel = xplSelectNodesInner(doc->xpath_ctxt, oldElement, select_attr);
				if (sel)
				{
					if (sel->type == XPATH_NODESET)
					{
						if (sel->nodesetval)
						{
							tail = NULL;
							for (j = 0; j < sel->nodesetval->nodeNr; j++)
							{	
								cloned = cloneAttrAsText(sel->nodesetval->nodeTab[j], oldElement);
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
				xplDisplayMessage(xplMsgWarning, BAD_CAST "missing macro content \"%s\" (file \"%s\", line %d)", select_attr, oldElement->doc->URL, oldElement->line);
			xmlFree(select_attr);
		} else { /* if (select_attr) */
			new_content = cloneNodeList(oldElement->children, cur->parent, oldElement->doc);
			if (required && !new_content)
				xplDisplayMessage(xplMsgWarning, BAD_CAST "missing macro content (file \"%s\", line %d)", oldElement->doc->URL, oldElement->line);
		}
		if (ret == cur)
			ret = new_content; /* <xpl:define name="A"><xpl:content/></xpl:define> */
		/* если выполнять замену сразу - сломается кэширование: предыдущее содержимое уже встроено в документ, и клонирование вернёт лишнее */
		cur->_private = new_content;
    }
	for (i = 0; i < content_cmds->nodeNr; i++) /* выполним саму замену */
	{
		cur = content_cmds->nodeTab[i];
		replaceWithList(cur, (xmlNodePtr) cur->_private);
		xmlFreeNode(cur);
	}
	if (content_cache)
		xmlHashFree(content_cache, NULL);
	xmlXPathFreeNodeSet(content_cmds);
	return ret;
}

/* node mode-based processing */
static void xplMacroParserApply(xplDocumentPtr doc, xmlNodePtr element, xplMacroPtr macro, xplResultPtr result);
static void xplNameParserApply(xplDocumentPtr doc, xmlNodePtr element, bool expand, xplResultPtr result);
static xplMacroPtr xplNodeHasMacro(xplDocumentPtr doc, xmlNodePtr element);

void xplNodeApply(xplDocumentPtr doc, xmlNodePtr element, bool expand, xplResultPtr result)
{
	xplMacroPtr macro;
	xmlHashTablePtr macros;
	
    if (expand && (macro = xplNodeHasMacro(doc, element)))
	{
		macro->times_encountered++;
        if (!macro->disabled_spin)
			xplMacroParserApply(doc, element, macro, result);     
		else
			xplNodeListApply(doc, element->children, expand, result); /* CopyParserApply */
	} else if (xplCheckNodeForXplNs(doc, element))
        xplNameParserApply(doc, element, expand, result);
    else
		xplNodeListApply(doc, element->children, expand, result); /* CopyParserApply */
	if ((macros = (xmlHashTablePtr) element->_private))
	{
		xplMacroTableFree(macros);
		element->_private = NULL;
	}
}

void xplNodeListApply(xplDocumentPtr doc, xmlNodePtr children, bool expand, xplResultPtr result)
{
	xmlNodePtr tail, c = children;

	while (c)
	{
		if (c->type == XML_ELEMENT_NODE)
		{
			/* xpl:break отсюда ушёл :) */
			xplNodeApply(doc, c, expand, result);
			if (!c->parent || (c->parent->type & XML_NODE_DELETION_MASK))
			{
				/* родитель удалён изнутри, немедленное прерывание */
				c->type = (xmlElementType) ((int) c->type & ~XML_NODE_DELETION_MASK);
				if (result->has_list)
					xmlFreeNodeList(result->list); /* не пригодится */
				break;
			}
			tail = c->next;
			if (c->type & XML_NODE_DELETION_MASK)
			{
				if (!(c->type & XML_NODE_DELETION_DEFERRED_FLAG))
				{
					c->type = (xmlElementType) ((int) c->type & ~XML_NODE_DELETION_REQUEST_FLAG);
					xmlUnlinkNode(c);
					xmlFreeNode(c); /* с детьми */
				} else
					; /* если установлен флажок DELETION_DEFERRED - не здесь, оно удалится само в конце обработки */
			} else if (result->has_list) { /* если список создан, */
				if (result->list && result->repeat) /* в нём существуют узлы и заказан повтор */
						tail = result->list;
				replaceWithList(c, result->list);
				/* xpl:delete, удаляющий сам себя, например */
				c->type = (xmlElementType) ((int) c->type & ~XML_NODE_DELETION_MASK);
				xmlFreeNode(c);
			} /* Иначе ничего не делаем */
		} else if (c->type == XML_TEXT_NODE) {
			tail = c->next;
			if (!strNonblank(c->content) && !doc->indent_spinlock) /* удаляем форматирование, если явно не запрошено обратное */
			{
				xmlUnlinkNode(c);
				xmlFreeNode(c);
			}
		} else if (c->type == XML_COMMENT_NODE) {
			/* незачем показывать юзеру наши безобразные комментарии */
			tail = c->next;
			xmlUnlinkNode(c);
			xmlFreeNode(c);
		} else 
			/* DTD, PI и иже с ними - скопировать в выходной поток */
			tail = c->next;
		c = tail;
	}
	ASSIGN_RESULT(NULL, false, false);
}

void xplMacroParserApply(xplDocumentPtr doc, xmlNodePtr element, xplMacroPtr macro, xplResultPtr result)
{
	xmlNodePtr out, prev_caller;
	xplMacroPtr prev_macro;

	doc->recursion_level++;
	if (doc->recursion_level > cfgMaxRecursionDepth)
	{
		ASSIGN_RESULT(xplCreateErrorNode(element, BAD_CAST "recursion depth exhausted (infinite loop in macro?..)"), true, true);
		return;
	}
	prev_macro = doc->current_macro;
	prev_caller = macro->caller;
	macro->caller = element;
	doc->current_macro = macro;
	macro->times_called++;
	if (element->children)
	{
		if (cfgWarnOnExpandedMacroContent && (macro->expansion_state == XPL_MACRO_EXPANDED))
		{
			if (element->ns && element->ns->prefix)
				xplDisplayMessage(xplMsgWarning, BAD_CAST "child nodes present in expanded macro caller \"%s:%s\" (file \"%s\", line %d)",
				element->ns->prefix, element->name, doc->document->URL, element->line);
			else
				xplDisplayMessage(xplMsgWarning, BAD_CAST "child nodes present in expanded macro caller \"%s\" (file \"%s\", line %d)",
				element->name, doc->document->URL, element->line);
		}
	}
	switch (macro->expansion_state)
	{
	case XPL_MACRO_EXPAND_ALWAYS:
	case XPL_MACRO_EXPAND_ONCE:
		/* возможно, где-то будет вызвана xpl:inherit - прибережём содержимое. намеренно не сбрасываем ему родителя - т.к. он может возвращаться */
		macro->node_original_content = element->children; 
		out = xplReplaceContentEntries(doc, macro->id, element, macro->content);
		setChildren(element, out);
		xplNodeListApply(doc, element->children, true, result);
		downshiftNodeListNsDef(out, element->nsDef);
		out = detachContent(element);
		if (!out) /* содержимое могло быть удалено командой return */
			out = macro->return_value;
		break;
	case XPL_MACRO_EXPANDED:
		out = cloneNodeList(macro->content, element->parent, element->doc);
		break;
	default:
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		out = xmlNewDocText(doc->document, BAD_CAST "internal error");
	}
	if (macro->expansion_state == XPL_MACRO_EXPAND_ONCE)
	{
		xmlFreeNodeList(macro->content);
		macro->content = cloneNodeList(out, element->parent, element->doc);
		macro->expansion_state = XPL_MACRO_EXPANDED;
	}
	ASSIGN_RESULT(out, false, true);
	doc->recursion_level--;
	doc->current_macro = prev_macro;
	macro->caller = prev_caller;
	macro->return_value = NULL; 
	if (macro->node_original_content)
	{
		xmlFreeNodeList(macro->node_original_content);
		macro->node_original_content = NULL;
	}
}

void xplNameParserApply(xplDocumentPtr doc, xmlNodePtr element, bool expand, xplResultPtr result)
{
	xmlNodePtr content; 
	xplCommandPtr cmd = NULL;
	xplCommandInfo cmdInfo;

	if (expand && !xmlStrcmp(element->name, BAD_CAST "define"))
	{ 
		content = xplAddMacro(doc, element, element->parent, false, XPL_MACRO_EXPAND_NO_DEFAULT, true); /* content - буфер ошибки */
		ASSIGN_RESULT(content, content? true: false, true);
	} else if (!xmlStrcmp(element->name, BAD_CAST "no-expand")) {
		xplNodeListApply(doc, element->children, false, result);
		content = detachContent(element);
		ASSIGN_RESULT(content, false, true);
	} else if (!xmlStrcmp(element->name, BAD_CAST "expand")) {
		xplNodeListApply(doc, element->children, true, result);
		content = detachContent(element);
		ASSIGN_RESULT(content, false, true);
	} else if (!xmlStrcmp(element->name, BAD_CAST "expand-after")) {
		xplNodeListApply(doc, element->children, false, result);
		xplNodeListApply(doc, element->children, true, result);
		content = detachContent(element);
		ASSIGN_RESULT(content, false, true);
	} else {
		if (expand)
		{
			cmd = xplGetCommand(element);
			if (cmd)
			{
				cmdInfo.element = element;
				cmdInfo.document = doc;
				cmdInfo._private = NULL;
				cmd->prologue(&cmdInfo); 
				/* элемент мб удалён из пролога (with) */
				if (!(element->type & XML_NODE_DELETION_MASK))
					xplNodeListApply(doc, cmdInfo.element->children, expand, result);
				/* дети могли похерить эл-т */
				if ((!(element->type & XML_NODE_DELETION_MASK)) || (cmd->flags & XPL_CMD_FLAG_CONTENT_SAFE))
					cmd->epilogue(&cmdInfo, result);
			} else {
				if (cfgWarnOnUnknownCommand)
					xplDisplayMessage(xplMsgWarning, BAD_CAST "unknown command \"%s\" (file \"%s\", line %d)", element->name, element->doc->URL, element->line);
				xplNodeListApply(doc, element->children, expand, result);
			} 
		} else 
			xplNodeListApply(doc, element->children, expand, result);
	}
}

xplMacroPtr xplNodeHasMacro(xplDocumentPtr doc, xmlNodePtr element)
{
	return xplMacroLookup(element->parent, element->ns? element->ns->href: NULL, element->name);
}

xmlNodePtr xplAddMacro(
	xplDocumentPtr doc, 
	xmlNodePtr macro, 
	xmlNodePtr destination, 
	bool fromNonCommand, 
	xplMacroExpansionState defaultExpansionState,
	bool defaultReplace
)
{
	xmlHashTablePtr macros; 
	xmlChar *name_attr = NULL;
	xmlChar *id_attr = NULL;
	xmlChar *expand_attr = NULL;
	xplMacroExpansionState expansion_state;
	bool replace = true;
	xplMacroPtr mb, prev_def = NULL, prev_macro;
	xmlChar *tagname;
	xmlNsPtr ns;
	xmlNodePtr ret = NULL;
	xplResult tmp_result;

	if (fromNonCommand)
	{
		tagname = name_attr = xmlStrdup(macro->name);
		ns = macro->ns;
	} else {
		name_attr = xmlGetNoNsProp(macro, BAD_CAST "name");
		if (!name_attr)
			return xplCreateErrorNode(macro, BAD_CAST "missing name attribute");
		EXTRACT_NS_AND_TAGNAME(name_attr, ns, tagname, macro)
	}
	if (xmlHasProp(macro, BAD_CAST "replace"))
	{
		if ((ret = xplDecodeCmdBoolParam(macro, BAD_CAST "replace", &replace, true)))
		{
			xmlFree(name_attr);
			return ret;
		}
	} else
		replace = defaultReplace;
	if (replace && cfgWarnOnMacroRedefinition) /* Отладочная проверка */
	{
		if ((prev_def = xplMacroLookup(macro->parent, ns? ns->href: NULL, tagname)))
			xplDisplayMessage(xplMsgWarning, BAD_CAST "macro \"%s\" redefined, previous line: %d (file \"%s\", line %d)", name_attr, prev_def->line, doc->document->URL, macro->line);
	}
	if (!replace && xplMacroLookup(macro->parent, ns? ns->href: NULL, tagname))
	{
		xmlFree(name_attr);
		return NULL;
	}
	id_attr = xmlGetNoNsProp(macro, BAD_CAST "id");
	expand_attr = xmlGetNoNsProp(macro, BAD_CAST "expand");
	if (expand_attr)
	{
		expansion_state = xplMacroExpansionStateFromString(expand_attr, false);
		if (expansion_state == XPL_MACRO_EXPAND_UNKNOWN)
		{
			ret = xplCreateErrorNode(macro, BAD_CAST "invalid expand attribute: \"%s\"", expand_attr);
			goto done;
		}
	} else if (defaultExpansionState != XPL_MACRO_EXPAND_NO_DEFAULT)
		expansion_state = defaultExpansionState;
	else
		expansion_state = XPL_MACRO_EXPAND_ALWAYS;
	mb = xplMacroCreate(id_attr, NULL, expansion_state);
	if (expansion_state == XPL_MACRO_EXPANDED)
	{
		prev_macro = doc->current_macro;
		doc->current_macro = mb;
		mb->caller = macro;
		xplNodeListApply(doc, macro->children, true, &tmp_result);
		doc->current_macro = prev_macro;
	}
	mb->content = detachContent(macro);
	if (!mb->content)
	{
		mb->content = mb->return_value;
		mb->return_value = NULL;
	}
	downshiftNodeListNsDef(mb->content, macro->nsDef);
	macros = (xmlHashTablePtr) destination->_private;
	if (!macros)
	{
		macros = xmlHashCreate(cfgInitialMacroTableSize);
		destination->_private = macros;
	}
	/* Имя узла - ПЕРВЫЙ параметр */
	if (xmlHashAddEntry2(macros, tagname, ns? ns->href: NULL, mb) == -1) /* уже есть */
		xmlHashUpdateEntry2(macros, tagname, ns? ns->href: NULL, mb, xplMacroDeallocator);
	/* Запишем информационные поля */
	mb->name = xmlStrdup(tagname);
	mb->ns = ns;
	ns = macro->nsDef;
	while (ns)
	{
		if (mb->ns == ns) /* ns на самой команде */
		{
			mb->ns_is_duplicated = true;
			mb->ns = xmlCopyNamespace(ns);
		}
		ns = ns->next;
	}
	mb->line = macro->line;
	mb->parent = destination;
done:
	if (name_attr) xmlFree(name_attr);
	if (id_attr) xmlFree(id_attr);
	if (expand_attr) xmlFree(expand_attr);
	return ret;
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
			return; /* Всё чисто */
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
			xplDisplayMessage(xplMsgWarning, BAD_CAST "XPL namespace uri \"%s\" differs from configured \"%s\", document \"%s\"",
			doc->root_xpl_ns->href, cfgXplNsUri, doc->document->URL);
		else
			xplDisplayMessage(xplMsgWarning, BAD_CAST "Cannot find XPL namespace on the root document element, document \"%s\"", doc->document->URL);
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
		root_element = doc->document->children;
		while (root_element && root_element->type != XML_ELEMENT_NODE)
			root_element = root_element->next;
		if (root_element)
		{
			xplCheckRootNs(doc, root_element);
			xplNodeApply(doc, root_element, true, &res);
#ifdef _THREADING_SUPPORT
			if (doc->threads)
			{
				doc->discard_suspended_threads = true;
				xplStartDelayedThreads(doc);
				xplWaitForChildThreads(doc);
				rbFreeBuf(doc->threads);
				doc->threads = NULL;
			}
#endif
			if (doc->fatal_content) /* Была вызвана xpl:fatal */
			{
				root_element->type = XML_ELEMENT_NODE;
				xplDeleteDeferredNodes(doc->deleted_nodes);
				xmlFreeNodeList(doc->document->children); /* списки не пересекаются, DDN вызывает xmlUnlinkNode() */
				doc->document->children = doc->document->last = doc->fatal_content;
				doc->fatal_content->parent = (xmlNodePtr) doc->document;
				ret = doc->status = XPL_ERR_FATAL_CALLED;
			} else {
				xplDeleteDeferredNodes(doc->deleted_nodes);
				ret = doc->status = XPL_ERR_NO_ERROR;
			}				
		} /* if (root_element) */
	} else {
		error_doc = xmlNewDoc(BAD_CAST "1.0");
		error_node = xmlNewDocNode(error_doc, NULL, ERROR_NODE_NAME, doc->error);
		error_doc->children = error_node;
		doc->document = error_doc;
		ret = doc->status = XPL_ERR_INVALID_DOCUMENT;
	}
	if (cfgDebugSaveFile && !doc->parent) /* Нет смысла сохранять содержимое порождённых документов */
	{
		if (!saveXmlDocToFile(doc->document, cfgDebugSaveFile, (char*) cfgDefaultEncoding, XML_SAVE_FORMAT))
			xplDisplayMessage(xplMsgWarning, BAD_CAST "Cannot save debug output to \"%s\", check that the file location exists and is writable", cfgDebugSaveFile);
	}
	return ret;
}

bool xplReadConfig()
{
	xmlDocPtr cfg;
	xmlNodePtr cur;
	int options_read = 0;

	if (!config_file_path)
		return 0;
	cfg = xmlParseFile((const char*) config_file_path); /* TODO path encoding */
	if (!cfg)
		return 0;
	cur = cfg->children->children;
	while (cur)
	{
		if (cur->type == XML_ELEMENT_NODE)
		{
			if (!xmlStrcmp(cur->name, BAD_CAST "Databases"))
			{
				if (!xplReadDatabases(cur))
				{
					xmlFreeDoc(cfg);
					return false;
				}
			} else if (!xmlStrcmp(cur->name, BAD_CAST "Options")) {
				xplReadOptions(cur);
				options_read = 1;
			} else if (!xmlStrcmp(cur->name, BAD_CAST "WrapperMap"))
				xplReadWrapperMap(cur);
		}
		cur = cur->next;
	}
	xmlFreeDoc(cfg);
	if (!options_read)
		xplAssignDefaultsToAllOptions();
	if (cfgCheckDbOnStartup)
		xplCheckDatabases();
	return true;
}

xplError xplInitParser(xmlChar *cfgFile)
{
	if (parser_loaded)
		return XPL_ERR_NO_ERROR;
	if (xplLoadableModulesInit() != XPL_MODULE_CMD_OK)
		return XPL_ERR_NO_PARSER;
	if (!cfgFile)
		return XPL_ERR_NO_CONFIG_FILE;
	config_file_path = xmlStrdup(cfgFile);
	if (!xplReadConfig())
	{
		parser_loaded = true;
		xplDoneParser();
		return XPL_ERR_NO_CONFIG_FILE;
	}
	if (!xplInitMessages())
	{
		parser_loaded = true;
		xplDoneParser();
		return XPL_ERR_NO_PARSER;
	}
	if (!xplInitCommands())
	{
		parser_loaded = true;
		xplDoneParser();
		return XPL_ERR_NO_PARSER;
	}
	if (!xplRegisterBuiltinCommands())
	{
		parser_loaded = true;
		xplDoneParser();
		return XPL_ERR_NO_PARSER;
	}
	initNamePointers();
	xplSessionManagerInit(cfgSessionLifetime);
	parser_loaded = true;
	return XPL_ERR_NO_ERROR;
}

void xplDoneParser()
{
	if (!parser_loaded)
		return;
	xplLoadableModulesCleanup();
	xplCleanupCommands();
	xplCleanupDatabases();
	xplCleanupOptions();
	xplCleanupWrapperMap();
	if (config_file_path)
	{
		xmlFree(config_file_path);
		config_file_path = NULL;
	}
	xplSessionManagerCleanup();
	xplCleanupMessages();
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
		xmlFree(cfgDocRoot);
	/* убравший отсюда дублирование строки потратит часа два-три на поиски причины вылета. */
	cfgDocRoot = xmlStrdup(new_root);
}

xmlChar* xplFullFilename(const xmlChar* file, const xmlChar* appPath)
{
	xmlChar* ret;
	int len;
	if (file && xmlStrlen(file) && ((*file == '/') || (*file == '\\'))) /* от корня системы */
	{
		appPath = xplGetDocRoot();
	}
	len = xmlStrlen(appPath) + xmlStrlen(file) + 1;
	ret = (xmlChar*) xmlMalloc(len);
	*ret = 0;
	if (appPath)
		strcpy((char*) ret, (const char*) appPath);
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
			/* Сбой: два текстовых узла подряд. */
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

xplError xplProcessFile(xmlChar *aAppPath, xmlChar *aFilename, xplParamsPtr aEnvironment, xplSessionPtr aSession, xplDocumentPtr *docOut)
{
	xplError ret;

	*docOut = xplDocumentCreateFromFile(aAppPath, aFilename, aEnvironment, aSession);
	if (!*docOut)
		return XPL_ERR_DOC_NOT_CREATED;
	(*docOut)->main = *docOut;
	(*docOut)->source = XPL_DOC_SOURCE_ORIGINAL;
	(*docOut)->status = XPL_ERR_NOT_YET_REACHED;
	(*docOut)->role = XPL_DOC_ROLE_MAIN;
	ret = xplDocumentApply(*docOut);
	if (ret >= XPL_ERR_NO_ERROR)
	{
		CHECK_COALESCING((*docOut)->document->children);
	} 
	return ret;
}

xplError xplProcessFileEx(xmlChar *basePath, xmlChar *relativePath, xplParamsPtr environment, xplSessionPtr session, xplDocumentPtr *docOut)
{
	xmlChar *prologue_file, *main_file, *epilogue_file;
	xmlChar *norm_path;
	xmlChar *norm_filename;
	xplError prologue_status, main_status = XPL_ERR_NOT_YET_REACHED, epilogue_status;
	xplDocumentPtr doc_prologue = NULL, doc_main = NULL, doc_epilogue = NULL;
	xplDocSource main_source;

	if (!cfgUseWrappers) 
	{ 
		composeAndSplitPath(basePath, relativePath, &norm_path, &norm_filename);
		main_status = xplProcessFile(norm_path, norm_filename, environment, session, docOut);
		if (norm_path) xmlFree(norm_path);
		return main_status;
	}

	/* Придётся заранее создать все обвязки, не вычисляя их - некоторые команды на это рассчитывают.
	   На всякий случай присвоим ссылки "prologue-main-epilogue" как декартово произведение. 
	 */
	prologue_file = xplMapDocWrapper(relativePath, XPL_DOC_ROLE_PROLOGUE);
	main_file = xplMapDocWrapper(relativePath, XPL_DOC_ROLE_MAIN);
	if (main_file)
		main_source = XPL_DOC_SOURCE_MAPPED;
	else {
		main_file = relativePath;
		main_source = XPL_DOC_SOURCE_ORIGINAL;
	}
	epilogue_file = xplMapDocWrapper(relativePath, XPL_DOC_ROLE_EPILOGUE);
	/* Пролог */
	if (prologue_file)
	{
		composeAndSplitPath(basePath, prologue_file, &norm_path, &norm_filename);
		doc_prologue = xplDocumentCreateFromFile(norm_path, norm_filename, environment, session);
		if (doc_prologue) 
		{
			doc_prologue->role = XPL_DOC_ROLE_PROLOGUE;
			doc_prologue->source = XPL_DOC_SOURCE_MAPPED;
			doc_prologue->prologue = doc_prologue;
		} else
			prologue_status = XPL_ERR_DOC_NOT_CREATED;
		if (norm_path) xmlFree(norm_path);
	}
	/* Основной файл */
	composeAndSplitPath(basePath, main_file, &norm_path, &norm_filename);
	doc_main = xplDocumentCreateFromFile(norm_path, norm_filename, environment, session);
	if (doc_main)
	{
		doc_main->role = XPL_DOC_ROLE_MAIN;
		doc_main->source = main_source;
		doc_main->prologue = doc_prologue;
		if (doc_prologue)
			doc_prologue->main = doc_main;
		doc_main->main = doc_main;
	} else
		main_status = XPL_ERR_DOC_NOT_CREATED;
	if (norm_path) xmlFree(norm_path);
	/* Эпилог */
	if (epilogue_file)
	{
		composeAndSplitPath(basePath, epilogue_file, &norm_path, &norm_filename);
		doc_epilogue = xplDocumentCreateFromFile(norm_path, norm_filename, environment, session);
		if (doc_epilogue) 
		{
			doc_epilogue->role = XPL_DOC_ROLE_EPILOGUE;
			doc_epilogue->source = XPL_DOC_SOURCE_MAPPED;
			doc_epilogue->prologue = doc_prologue;
			doc_epilogue->main = doc_main;
			doc_epilogue->epilogue = doc_epilogue;
			if (doc_prologue)
				doc_prologue->epilogue = doc_epilogue;
			if (doc_main)
				doc_main->epilogue = doc_epilogue;
		} else
			epilogue_status = XPL_ERR_DOC_NOT_CREATED;
		if (norm_path) xmlFree(norm_path);
	} 
	/* Теперь - собственно вычисление */
	if (doc_prologue)
	{
		prologue_status = xplDocumentApply(doc_prologue);
		doc_prologue->status = prologue_status;
	}
	if (doc_main)
	{
		if (doc_main->source == XPL_DOC_SOURCE_OVERRIDDEN) 
			main_status = prologue_status; /* вернём детерминированный статус */
		else {
			main_status = xplDocumentApply(doc_main);
			doc_main->status = main_status;
		}
	}
	if (doc_epilogue)
	{
		epilogue_status = xplDocumentApply(doc_epilogue);
		doc_epilogue->status = epilogue_status;
	}
	*docOut = doc_epilogue? doc_epilogue->main: doc_main;
	return doc_epilogue? doc_epilogue->main->status: main_status;
}


#if defined(_IN_DLL) && defined(_WIN32)
int __stdcall DllMain(int reason)
{
	switch(reason)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	}
	return 0;
}
#endif
