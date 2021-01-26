#include <libxpl/xpltree.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libxpl/abstraction/xpr.h>

xplParseQNameResult xplParseQName(xmlChar *str, xmlNodePtr element, xplQName *qname)
{
	xmlChar *prefix;

	if (xmlValidateQName(str, 0))
		return XPL_PARSE_QNAME_INVALID_QNAME;
	qname->ncname = xmlSplitQName2(str, &prefix);
	if (!qname->ncname) /* not a QName */
	{
		qname->ncname = str;
		qname->ns = NULL;
	} else {
		qname->ns = xmlSearchNs(element->doc, element, prefix);
		XPL_FREE(prefix);
		if (!qname->ns)
		{
			XPL_FREE(qname->ncname);
			qname->ncname = NULL;
			return XPL_PARSE_QNAME_UNKNOWN_NS;
		}
	}
	return XPL_PARSE_QNAME_OK;
}

xmlChar* xplQNameToStr(xplQName qname)
{
	size_t prefix_len;
	xmlChar *ret;

	if (qname.ns)
		prefix_len = xmlStrlen(qname.ns->prefix) + 1;
	else
		prefix_len = 0;
	ret = (xmlChar*) XPL_MALLOC(prefix_len + xmlStrlen(qname.ncname) + 1);
	if (qname.ns)
	{
		strcpy((char*) ret, (char*) qname.ns->prefix);
		ret[prefix_len] = (xmlChar) ':';
	}
	strcpy((char*) ret + prefix_len, (char*) qname.ncname);
	return ret;
}

void xplClearQName(xplQNamePtr qname)
{
	if (qname->ns && qname->ncname)
	{
		XPL_FREE(qname->ncname);
		qname->ncname = NULL;
	}
	qname->ns = NULL;
}


xmlNodePtr xplFindTail(xmlNodePtr cur)
{
	if (!cur)
		return NULL;
	while (cur->next)
		cur = cur->next;
	return cur;
}

bool xplIsAncestor(xmlNodePtr maybeChild, xmlNodePtr maybeAncestor)
{
	if (maybeChild == maybeAncestor)
		return false;
	if (!maybeChild || !maybeAncestor || !maybeChild->parent)
		return false;
	while (maybeChild)
	{
		if (maybeChild->parent == maybeAncestor)
			return true;
		if (maybeChild == maybeAncestor->parent)
			return false;
		maybeChild = maybeChild->parent;
	}
	return false;
}

/* TODO check for occurrences and possibly remove */
xmlNsPtr xplGetResultingNs(xmlNodePtr parent, xmlNodePtr invoker, xmlChar *name)
{
	xmlNsPtr ret;

	if ((ret = xmlSearchNs(parent->doc, parent, name)))
		return ret;
	if ((ret = xmlSearchNs(invoker->doc, invoker, name)))
		return xmlNewNs(parent, ret->href, ret->prefix);
	return NULL;
}

/* from libxml2 internals (tree.c) */
xmlChar* xplGetPropValue(xmlAttrPtr prop)
{
	if (!prop)
		return NULL;
	if (prop->type != XML_ATTRIBUTE_NODE)
		return NULL;
	if (!prop->children)
		return BAD_CAST XPL_STRDUP("");
	if (!prop->children->next && ((prop->children->type == XML_TEXT_NODE) || (prop->children->type == XML_CDATA_SECTION_NODE)))
		return BAD_CAST XPL_STRDUP((char*) prop->children->content);
	return xmlNodeListGetString(prop->doc, prop->children, 1);
}

void xplUnlinkProp(xmlAttrPtr cur)
{
	xmlAttrPtr tmp = cur->parent->properties;

	if (tmp == cur)
	{
		cur->parent->properties = cur->next;
		if (cur->next)
			cur->next->prev = NULL;
		return;
	}
	while (tmp)
	{
		if (tmp->next == cur)
		{
			tmp->next = cur->next;
			if (tmp->next)
				tmp->next->prev = tmp;
			return;
		}
		tmp = tmp->next;
	}
}

xmlNodePtr xplDetachContent(xmlNodePtr el)
{
	xmlNodePtr item;

	if (!el)
		return NULL;
	item = el->children;
	while (item)
	{
		item->parent = NULL;
		item = item->next;
	}
	item = el->children;
	el->children = el->last = NULL;
	return item;
}

static xmlNodePtr _setListParent(xmlNodePtr el, xmlNodePtr list)
{
	list->parent = el;
	while (list->next)
	{
		list = list->next;
		list->parent = el;
	}
	return list;
}

xmlNodePtr xplSetChildren(xmlNodePtr el, xmlNodePtr list)
{
	if (!el)
		return NULL;
	el->children = list;
	if (!list)
	{
		el->last = NULL;
		return NULL;
	}
	return el->last = _setListParent(el, list);
}

/* TODO all tree modifications must flatten adjacent text nodes */
xmlNodePtr xplAppendChildren(xmlNodePtr el, xmlNodePtr list)
{
	if (!list)
		return NULL;
	if (!el)
		return xplFindTail(list);
	if (el->children)
		return xplAppendList(el->last? el->last: xplFindTail(el->children), list);
	el->children = list;
	return el->last = _setListParent(el, list);
}

xmlNodePtr xplAppendList(xmlNodePtr el, xmlNodePtr list)
{
	xmlNodePtr tail;

	if (!list)
		return NULL;
	if (!el)
		return xplFindTail(list);
	tail = _setListParent(el->parent, list);
	tail->next = el->next;
	if (el->next)
		el->next->prev = tail;
	el->next = list;
	list->prev = el;
	if (el->parent && (el->parent->last == el))
		el->parent->last = tail;
	return tail;
}

xmlNodePtr xplPrependList(xmlNodePtr el, xmlNodePtr list)
{
	xmlNodePtr tail;

	if (!list)
		return NULL;
	if (!el)
		return xplFindTail(list);
	list->prev = el->prev;
	if (el->prev)
		el->prev->next = list;
	tail = _setListParent(el->parent, list);
	el->prev = tail;
	tail->next = el;
	if (el->parent && (el->parent->children == el))
		el->parent->children = list;
	return tail;
}

xmlNodePtr xplReplaceWithList(xmlNodePtr el, xmlNodePtr list)
{
	xmlNodePtr tail;

	if (!el)
		return NULL;
	if (!list)
	{
		xmlUnlinkNode(el);
		return el;
	}
	if (el->prev)
		el->prev->next = list;
	list->prev = el->prev;
	tail = _setListParent(el->parent, list);
	if (el->next)
		el->next->prev = tail;
	tail->next = el->next;
	if (el->parent && el->parent->children == el)
		el->parent->children = list;
	if (el->parent && el->parent->last == el)
		el->parent->last = tail;
	el->prev = el->next = el->parent = NULL;
	return el;
}

xmlNodePtr xplCloneAsNodeChild(xmlNodePtr cur, xmlNodePtr parent)
{
	xmlNodePtr ret;

	if (!cur)
		return NULL;
	if (cur->type == XML_ATTRIBUTE_NODE)
	{
		if (cur->children)
		{
			if (!cur->children->next && ((cur->children->type == XML_TEXT_NODE) || (cur->children->type == XML_CDATA_SECTION_NODE)))
				ret = xmlNewDocText(parent->doc, cur->children->content);
			else {
				ret = xmlNewDocText(parent->doc, NULL);			
				ret->content = xmlNodeListGetString(parent->doc, cur->children, 1);
			}
		} else 
			ret = xmlNewDocText(parent->doc, BAD_CAST "");
	} else {
		ret = xplCloneNode(cur, parent, parent->doc);
	}
	return ret;
}

bool xplCheckNodeListForText(xmlNodePtr start)
{
	while (start)
	{
		if ((start->type != XML_TEXT_NODE) 
			&& (start->type != XML_ENTITY_REF_NODE)
			&& (start->type != XML_CDATA_SECTION_NODE)
		)
			return false;
		start = start->next;
	}
	return true;
}

bool xplCheckNodeSetForText(xmlNodeSetPtr s)
{
	size_t i;

	if (!s)
		return false;
	for (i = 0; i < (size_t) s->nodeNr; i++)
		if ((s->nodeTab[i]->type != XML_TEXT_NODE) 
			&& (s->nodeTab[i]->type != XML_ATTRIBUTE_NODE)
			&& (s->nodeTab[i]->type != XML_ENTITY_REF_NODE)
			&& (s->nodeTab[i]->type != XML_CDATA_SECTION_NODE)
		)
			return false;
	return true;
}

void xplMarkAncestorAxisForDeletion(xmlNodePtr bottom, xmlNodePtr top)
{
	if (!bottom || !top)
		return;
	while (true)
	{
		bottom->type = (xmlElementType) ((int) bottom->type | XPL_NODE_DELETION_REQUEST_FLAG);
		if (bottom == top)
			break;
		bottom = bottom->parent;
	} 
}

void xplMarkDOSAxisForDeletion(xmlNodePtr cur, int bitwiseAttribute, bool doMark)
{
	xmlAttrPtr prop;

	cur->type = (xmlElementType) doMark
		? ((int) cur->type |  bitwiseAttribute)
		: ((int) cur->type & ~bitwiseAttribute);
	prop = cur->properties;
	while (prop)
	{
		prop->type = (xmlElementType) doMark
			? ((int) cur->type |  bitwiseAttribute)
			: ((int) cur->type & ~bitwiseAttribute);
		prop = prop->next;
	}
	cur = cur->children;
	while (cur)
	{
		xplMarkDOSAxisForDeletion(cur, bitwiseAttribute, doMark);
		cur = cur->next;
	}
}

void xplDeleteNeighbours(xmlNodePtr cur, xmlNodePtr boundary, bool markAncestorAxis)
{
	if  (!cur || !boundary) 
		return;
	while (cur != boundary)
	{
		if (cur->prev)
		{
			cur->prev->next = NULL;
			xmlFreeNodeList(cur->parent->children);
			cur->prev = NULL;
		}
		if (cur->next)
		{
			cur->next->prev = NULL;
			xmlFreeNodeList(cur->next);
			cur->next = NULL;
		}
		cur->parent->children = cur->parent->last = cur;
		if (markAncestorAxis)
			cur->type = (xmlElementType) ((int) cur->type | XPL_NODE_DELETION_REQUEST_FLAG);
		cur = cur->parent;
	}
}

/* a lot of useful libxml2 functions is made static to avoid namespace
   definition transfers between documents - so we have to copy them here. */
#define UPDATE_LAST_CHILD_AND_PARENT(n) \
if ((n))								\
{										\
    xmlNodePtr ulccur = (n)->children;	\
	if (!ulccur)						\
        (n)->last = NULL;				\
    else {								\
        while (ulccur->next != NULL)	\
		{								\
			ulccur->parent = (n);		\
			ulccur = ulccur->next;		\
		}								\
		ulccur->parent = (n);			\
		(n)->last = ulccur;				\
	}									\
}

/* pointers to static node names are hidden and have to be re-initialized */

static xmlChar* xmlStringText;
static xmlChar* xmlStringComment;

bool xplInitNamePointers()
{
	xmlNodePtr cur = xmlNewText(NULL);
	if (!cur)
		return false;
	xmlStringText = BAD_CAST cur->name;
	XPL_FREE(cur);
	cur = xmlNewComment(NULL);
	if (!cur)
		return false;
	xmlStringComment = BAD_CAST cur->name;
	XPL_FREE(cur);
	return true;
}

#define MAX_PREFIX_SIZE 32
static xmlChar*  _newNsPrefix(xmlDocPtr doc, xmlNodePtr tree, xmlNsPtr ns)
{
	xmlNsPtr def;
	xmlChar *prefix;
	int counter = 1;

	prefix = BAD_CAST XPL_MALLOC(MAX_PREFIX_SIZE);
	if (!prefix)
		return NULL;

    if (!ns->prefix)
		snprintf((char*) prefix, MAX_PREFIX_SIZE, "default");
    else
		snprintf((char*) prefix, MAX_PREFIX_SIZE, "%.20s", (char*) ns->prefix);

    def = xmlSearchNs(doc, tree, prefix);
    while (def)
	{
        if (counter > 1000)
        {
        	XPL_FREE(prefix);
			return NULL;
        }
		if (!ns->prefix)
			snprintf((char*) prefix, MAX_PREFIX_SIZE, "default%d", counter++);
		else
			snprintf((char*) prefix, MAX_PREFIX_SIZE, "%.20s%d", (char *)ns->prefix, counter++);
		def = xmlSearchNs(doc, tree, prefix);
    }
    return prefix;
}
#undef MAX_PREFIX_SIZE

static xmlNsPtr newReconciliedNs(xmlDocPtr doc, xmlNodePtr tree, xmlNsPtr ns)
{
    xmlNsPtr def;
    xmlChar *prefix;

    if (!tree)
		return NULL;
    if ((!ns) || (ns->type != XML_NAMESPACE_DECL)) 
		return NULL;
    /*
     * Search an existing namespace definition inherited.
     */
    def = xmlSearchNsByHref(doc, tree, ns->href);
    if (def)
        return def;

    /*
     * Find a close prefix which is not already in use.
     * Let's strip namespace prefixes longer than 20 chars !
     */
    if (!(prefix = _newNsPrefix(doc, tree, ns)))
    	return NULL;
    /*
     * OK, now we are ready to create a new one.
     */
    def = xmlNewNs(tree, ns->href, NULL);
    def->prefix = prefix;
    return def;
}

static xmlAttrPtr clonePropInternal(xmlDocPtr doc, xmlNodePtr target, xmlAttrPtr cur, xmlNodePtr top_clone)
{
    xmlAttrPtr ret;

    if (!cur) 
		return NULL;
    if (target)
		ret = xmlNewDocProp(target->doc, cur->name, NULL);
    else if (doc)
		ret = xmlNewDocProp(doc, cur->name, NULL);
    else if (cur->parent)
		ret = xmlNewDocProp(cur->parent->doc, cur->name, NULL);
    else if (cur->children)
		ret = xmlNewDocProp(cur->children->doc, cur->name, NULL);
    else
		ret = xmlNewDocProp(NULL, cur->name, NULL);
    if (!ret) 
		return NULL;
    ret->parent = target;

    if (cur->ns && target) 
	{
		xmlNsPtr ns;
		ns = xmlSearchNs(target->doc, target, cur->ns->prefix);
		if (!ns) 
		{
			/* Hmm, we are copying an attribute whose namespace is defined
			  out of the new tree scope. Search it in the original tree
			  and add it at the top of the new tree. */
			ns = xmlSearchNs(cur->doc, cur->parent, cur->ns->prefix);
			if (ns) 
			{
				xmlNodePtr root = target;
				while (root->parent) 
				{	
					if (root == top_clone)
						break;
					root = root->parent;
				}
				ret->ns = xmlNewNs(root, ns->href, ns->prefix);
			}
		} else {
	        /* we have to find something appropriate here since we can't be sure
	           that the namespace we found is identified by the prefix */
			if (xmlStrEqual(ns->href, cur->ns->href)) 
				/* this is the nice case */
				ret->ns = ns;
			else
				/* we are in trouble: we need a new reconciled namespace.
				   This is expensive. */
				ret->ns = newReconciliedNs(target->doc, target, cur->ns);
		}
    } else
        ret->ns = NULL;

    if (cur->children) 
		xplSetChildren((xmlNodePtr) ret, xplCloneNodeList(cur->children, (xmlNodePtr) ret, ret->doc));
    /*
     * Try to handle IDs
     */
	if (target && cur && target->doc && cur->doc &&	cur->doc->ids && cur->parent) 
	{
		if (xmlIsID(cur->doc, cur->parent, cur)) 
		{
			xmlChar *id;
		    id = xmlNodeListGetString(cur->doc, cur->children, 1);
		    if (id) 
			{
				xmlAddID(NULL, target->doc, id, ret);
				XPL_FREE(id);
			}
		}
    }
    return(ret);
}

static xmlAttrPtr clonePropListInner(xmlDocPtr doc, xmlNodePtr target, xmlAttrPtr cur, xmlNodePtr top_clone)
{
    xmlAttrPtr ret = NULL;
    xmlAttrPtr last = NULL, tmp;

    while (cur) 
	{
        tmp = clonePropInternal(doc, target, cur, top_clone);
		if (!tmp)
			return(NULL);
		if (!last)
		{
			ret = last = tmp;
		} else {
			last->next = tmp;
			tmp->prev = last;
			last = tmp;
		}
		cur = cur->next;
    }
    return(ret);
}

static xmlNodePtr cloneNodeListInner(xmlNodePtr node, xmlNodePtr parent, xmlDocPtr doc, xmlNodePtr top_clone);

static xmlNodePtr cloneNodeInner(xmlNodePtr node, xmlNodePtr parent, xmlDocPtr doc, xmlNodePtr top_clone)
{
    xmlNodePtr ret;

    if (!node) 
		return NULL;
    switch (node->type) 
	{
        case XML_TEXT_NODE:
        case XML_CDATA_SECTION_NODE:
        case XML_ELEMENT_NODE:
        case XML_DOCUMENT_FRAG_NODE:
        case XML_ENTITY_REF_NODE:
        case XML_ENTITY_NODE:
        case XML_PI_NODE:
        case XML_COMMENT_NODE:
        case XML_XINCLUDE_START:
        case XML_XINCLUDE_END:
        	break;
        case XML_ATTRIBUTE_NODE:
        	return (xmlNodePtr) clonePropInternal(doc, parent, (xmlAttrPtr) node, NULL);
        case XML_NAMESPACE_DECL:
        	return (xmlNodePtr) xmlCopyNamespaceList((xmlNsPtr) node);
        case XML_DOCUMENT_NODE:
        case XML_HTML_DOCUMENT_NODE:
#ifdef LIBXML_DOCB_ENABLED
        case XML_DOCB_DOCUMENT_NODE:
#endif
#ifdef LIBXML_TREE_ENABLED
        	return (xmlNodePtr) xmlCopyDoc((xmlDocPtr) node, 1);
#endif /* LIBXML_TREE_ENABLED */
        case XML_DOCUMENT_TYPE_NODE:
        case XML_NOTATION_NODE:
        case XML_DTD_NODE:
        case XML_ELEMENT_DECL:
        case XML_ATTRIBUTE_DECL:
        case XML_ENTITY_DECL:
            return(NULL);
    }

    /*
     * Allocate a new node and fill the fields.
     */
    ret = (xmlNodePtr) XPL_MALLOC(sizeof(xmlNode));
    if (!ret) 
		return NULL;
    memset(ret, 0, sizeof(xmlNode));
    ret->type = node->type;

    ret->doc = doc;
    ret->parent = parent;
	if (!top_clone)
		top_clone = ret;
    if (node->name == xmlStringText)
		ret->name = xmlStringText;
	/* this is only possible in libxslt */
	/*
    else if (node->name == xmlStringTextNoenc)
		ret->name = xmlStringTextNoenc;
	*/
    else if (node->name == xmlStringComment)
		ret->name = xmlStringComment;
    else if (node->name != NULL) 
	{
        if ((doc != NULL) && (doc->dict != NULL))
		    ret->name = xmlDictLookup(doc->dict, node->name, -1);
		else
			ret->name = BAD_CAST XPL_STRDUP((char*) node->name);
    }
    if ((node->type != XML_ELEMENT_NODE) &&
		(node->content != NULL) &&
		(node->type != XML_ENTITY_REF_NODE) &&
		(node->type != XML_XINCLUDE_END) &&
		(node->type != XML_XINCLUDE_START)) 
	{
		ret->content = BAD_CAST XPL_STRDUP((char*) node->content);
    } else {
      if (node->type == XML_ELEMENT_NODE)
        ret->line = node->line;
    }
    if (parent != NULL && ret != top_clone) 
	{
		xmlNodePtr tmp;

		/* we don't use this */
		/*
		if ((__xmlRegisterCallbacks) && (xmlRegisterNodeDefaultValue))
			xmlRegisterNodeDefaultValue((xmlNodePtr)ret);
		 */
        tmp = xmlAddChild(parent, ret);
		/* node could have coalesced */
		if (tmp != ret)
			return(tmp);
    }

    if (((node->type == XML_ELEMENT_NODE) ||
         (node->type == XML_XINCLUDE_START)) && (node->nsDef != NULL))
        ret->nsDef = xmlCopyNamespaceList(node->nsDef);

    if (node->ns) 
	{
        xmlNsPtr ns;
		ns = xmlSearchNs(doc, ret, node->ns->prefix);
		if (!ns) 
		{
			/*
			* Humm, we are copying an element whose namespace is defined
			* out of the new tree scope. Search it in the original tree
			* and add it at the top of the new tree
			*/
			ns = xmlSearchNs(node->doc, node, node->ns->prefix);
			if (ns) 
			{
				xmlNodePtr root = ret;
				while (root->parent) 
				{
					if (root == top_clone)
						break;
					else
						root = root->parent;
				}
				ret->ns = xmlNewNs(root, ns->href, ns->prefix);
			}
		} else {
			/*
			* reference the existing namespace definition in our own tree.
			*/
			ret->ns = ns;
		}
    }
    if (((node->type == XML_ELEMENT_NODE) ||
         (node->type == XML_XINCLUDE_START)) && (node->properties != NULL))
        ret->properties = clonePropListInner(doc, ret, node->properties, top_clone);
    if (node->type == XML_ENTITY_REF_NODE) 
	{
		if ((!doc) || (node->doc != doc)) 
		{
			/*
			* The copied node will go into a separate document, so
			* to avoid dangling references to the ENTITY_DECL node
			* we cannot keep the reference. Try to find it in the
			* target document.
			*/
			ret->children = (xmlNodePtr) xmlGetDocEntity(doc, ret->name);
		} else {
            ret->children = node->children;
		}
		ret->last = ret->children;
    } else if (node->children) {
        ret->children = cloneNodeListInner(node->children, ret, doc, top_clone);
		UPDATE_LAST_CHILD_AND_PARENT(ret)
    }

    /* we don't use this */
	/*
    if ((!parent) && ((__xmlRegisterCallbacks) && (xmlRegisterNodeDefaultValue)))
		xmlRegisterNodeDefaultValue((xmlNodePtr)ret);
	 */
	if (ret == top_clone)
		ret->parent = NULL;
    return(ret);
}

static xmlNodePtr cloneNodeListInner(xmlNodePtr node, xmlNodePtr parent, xmlDocPtr doc, xmlNodePtr top_clone)
{
    xmlNodePtr ret = NULL;
    xmlNodePtr p = NULL, q;

    while (node) 
	{
#ifdef LIBXML_TREE_ENABLED
		if (node->type == XML_DTD_NODE ) 
		{
			if (!doc) 
			{
				node = node->next;
				continue;
			}
			if (!doc->intSubset) 
			{
				q = (xmlNodePtr) xmlCopyDtd((xmlDtdPtr) node);
				q->doc = doc;
				q->parent = parent;
				doc->intSubset = (xmlDtdPtr) q;
				xmlAddChild(parent, q);
			} else {
				q = (xmlNodePtr) doc->intSubset;
				xmlAddChild(parent, q);
			}
		} else
#endif /* LIBXML_TREE_ENABLED */
			q = cloneNodeInner(node, parent, doc, top_clone);
		if (!ret) 
		{
			q->prev = NULL;
			ret = p = q;
		} else if (p != q) {
			/* the test is required if cloneNodeInner [xmlStaticCopyNode] coalesced 2 text nodes */
			p->next = q;
			q->prev = p;
			p = q;
		}
		node = node->next;
    }
    return(ret);
}

xmlNodePtr xplCloneNode(xmlNodePtr node, xmlNodePtr parent, xmlDocPtr doc)
{
	return cloneNodeInner(node, parent, doc, NULL);
}

xmlNodePtr xplCloneNodeList(xmlNodePtr node, xmlNodePtr parent, xmlDocPtr doc)
{
	return cloneNodeListInner(node, parent, doc, NULL);
}

static void _addNsDef(xmlNodePtr cur, xmlNsPtr def)
{
	xmlNsPtr old = cur->nsDef;

	cur->nsDef = def;
	if (old)
		cur->nsDef->next = old;
}

void xplDownshiftNodeNsDef(xmlNodePtr cur, xmlNsPtr ns_list)
{
	xmlNsPtr ns_cur, ns_to_shift;
	xmlAttrPtr prop;

	if (!cur || !ns_list)
		return;
	ns_cur = ns_list;
	if (cur->ns)
	{
		while (ns_cur)
		{
			if (ns_cur == cur->ns)
			{
				ns_to_shift = xmlSearchNs(cur->doc, cur, cur->ns->prefix);
				if (ns_to_shift)
					cur->ns = ns_to_shift;
				else {
					ns_to_shift = xmlCopyNamespace(cur->ns);
					cur->ns = ns_to_shift;
					_addNsDef(cur, ns_to_shift);
				}
				break;
			}
			ns_cur = ns_cur->next;
		}
	}
	/* check all attributes */
	prop = cur->properties;
	while (prop)
	{
		if (prop->ns)
		{
			ns_cur = ns_list;
			while (ns_cur)
			{
				if (prop->ns == ns_cur)
				{
					ns_to_shift = xmlSearchNs(cur->doc, cur, prop->ns->prefix);
					if (ns_to_shift)
						prop->ns = ns_to_shift;
					else {
						ns_to_shift = xmlCopyNamespace(prop->ns);
						prop->ns = ns_to_shift;
						_addNsDef(cur, ns_to_shift);
					}
					break;
				}
				ns_cur = ns_cur->next;
			}
		}
		prop = prop->next;
	}
	if (cur->children)
		xplDownshiftNodeListNsDef(cur->children, ns_list);
}

void xplDownshiftNodeListNsDef(xmlNodePtr cur, xmlNsPtr ns_list)
{
	if (!ns_list)
		return;
	while (cur)
	{
		xplDownshiftNodeNsDef(cur, ns_list);
		cur = cur->next;
	}
}

typedef struct _xplNsPair
{
	xmlNsPtr old_ns;
	xmlNsPtr new_ns;
} xplNsPair, *xplNsPairPtr;

typedef struct _xplNsPairs
{
	xplNsPairPtr namespaces;
	size_t count;
	size_t size;
} xplNsPairs, *xplNsPairsPtr;

static bool _initNsPairs(xplNsPairsPtr pairs, size_t initialSize)
{
	pairs->count = 0;
	pairs->size = initialSize;
	if (!(pairs->namespaces = (xplNsPairPtr) XPL_MALLOC(sizeof(xplNsPair) * initialSize)))
		return false;
	return true;
}
/*
static xplNsPairsPtr _createNsPairs(size_t initialSize)
{
	xplNsPairsPtr ret;

	assert(initialSize > 0);
	ret = (xplNsPairsPtr) XPL_MALLOC(sizeof(xplNsPairs));
	if (!ret)
		return NULL;
	if (!_initNsPairs(ret, initialSize))
	{
		XPL_FREE(ret);
		return NULL;
	}
	return ret;
}
*/
static void _clearNsPairs(xplNsPairsPtr pairs, bool clearNew)
{
	size_t i;

	if (clearNew)
		for (i = 0; i < pairs->count; i++)
			xmlFreeNs(pairs->namespaces[i].new_ns);
	if (pairs->namespaces)
		XPL_FREE(pairs->namespaces);
	pairs->namespaces = NULL;
	pairs->count = pairs->size = 0;
}
/*
static void _freeNsPairs(xplNsPairsPtr pairs)
{
	_clearNsPairs(pairs);
	XPL_FREE(pairs);
}
*/
static bool _addNamespacesToNsPairs(xplNsPairsPtr pairs, xmlNsPtr old, xmlNsPtr new)
{
	xplNsPairPtr new_namespaces;

	if (pairs->count == pairs->size)
	{
		/* don't realloc as we may lose pointers in case of failure */
		if (!(new_namespaces = (xplNsPairPtr) XPL_MALLOC(sizeof(xmlNsPtr) * pairs->size * 2)))
			return false;
		memcpy(new_namespaces, pairs->namespaces, sizeof(xmlNsPtr) * pairs->size);
		XPL_FREE(pairs->namespaces);
		pairs->namespaces = new_namespaces;
		pairs->size *= 2;
	}
	pairs->namespaces[pairs->count].old_ns = old;
	pairs->namespaces[pairs->count].new_ns = new;
	pairs->count++;
	return true;
}
/*
static bool _addNsPairToNsPairs(xplNsPairPtr pairs, xplNsPair pair)
{
	return _addNamespacesToNsPairs(pairs, pair.old_ns, pair.new_ns);
}
*/
static xmlNsPtr _getPairedNs(xplNsPairsPtr pairs, xmlNsPtr ns)
{
	size_t i;

	for (i = 0; i < pairs->count; i++)
		if (pairs->namespaces[i].old_ns == ns)
			return pairs->namespaces[i].new_ns;
	return ns;
}

static void _relinkTreeNamespaces(xplNsPairsPtr pairs, xmlNodePtr cur);

static void _relinkNodeListNamespaces(xplNsPairsPtr pairs, xmlNodePtr list)
{
	while (list)
	{
		_relinkTreeNamespaces(pairs, list);
		list = list->next;
	}
}

static void _relinkTreeNamespaces(xplNsPairsPtr pairs, xmlNodePtr cur)
{
	xmlAttrPtr attr;

	if (cur->type != XML_ELEMENT_NODE)
		return;
	if (cur->ns)
		cur->ns = _getPairedNs(pairs, cur->ns);
	attr = cur->properties;
	while (attr)
	{
		if (attr->ns)
			attr->ns = _getPairedNs(pairs, attr->ns);
		attr = attr->next;
	}
	_relinkNodeListNamespaces(pairs, cur->children);
}

bool xplMakeNsSelfContainedTree(xmlNodePtr top)
{
	xplNsPairs pairs;
	xmlNodePtr cur;
	xmlNsPtr old_ns, new_ns;

	if (!_initNsPairs(&pairs, 16))
		return false;
	cur = top->parent;
	while (cur->type == XML_ELEMENT_NODE)
	{
		old_ns = cur->nsDef;
		while (old_ns)
		{
			/* TODO we may want our own implementation of xmlNewNs:
			   it returns NULL if a namespace with the same prefix already exists
			   OR there's no memory to allocate it. */
			if ((new_ns = xmlNewNs(top, old_ns->href, old_ns->prefix)))
			{
				if (!_addNamespacesToNsPairs(&pairs, old_ns, new_ns)) // OOM
				{
					_clearNsPairs(&pairs, true);
					top->nsDef = NULL;
					return false;
				}
			}
			old_ns = old_ns->next;
		}
		cur = cur->parent;
	}
	if (pairs.count)
		_relinkNodeListNamespaces(&pairs, top->children);
	_clearNsPairs(&pairs, false);
	return true;
}

/* now the opposite task - getting rid of duplicated definitions */

void xplReplaceRedundantNamespaces(xmlNodePtr top)
{
	UNUSED_PARAM(top);
}

/* XPath extensions */
/* not now: some heavy funcs in libxml2 are static */
/* TODO this should be in xplcore anyway */
#if 0
void xplXPathIsDefinedFunction(xmlXPathParserContextPtr ctxt, int nargs) 
{
    xmlXPathObjectPtr val = NULL;
	xmlChar *what;
	int ret = 0;
	xplCommandInfoPtr info;
	xmlHashTablePtr macros;
	void *macro = NULL;

    CHECK_ARITY(1);
    CAST_TO_STRING;
    CHECK_TYPE(XPATH_STRING);
    val = valuePop(ctxt);
    what = val->stringval;
	if (ctxt->context->userData)
	{
		info = (xplCommandInfoPtr) ctxt->context->userData;
		macros = info->macroTable;
		while (!macro && macros)
		{
			macro = xmlHashLookup(macros, what);
			if (!macro)
				macros = (xmlHashTablePtr) xmlHashLookup(macros, PARENT_MACROS_NAME);
		}
		ret = (macro != NULL);
	}
    xmlXPathReleaseObject(ctxt->context, val);
    valuePush(ctxt, xmlXPathCacheNewBoolean(ctxt->context, ret));
}
#endif

void xplRegisterXPathExtensions(xmlXPathContextPtr ctxt)
{
#if 0
    xmlXPathRegisterFunc(ctxt, (const xmlChar *) "is-defined", xplXPathIsDefinedFunction);
#else
    UNUSED_PARAM(ctxt);
#endif
}

bool xplCheckNodeEquality(xmlNodePtr a, xmlNodePtr b)
{
	if (!a && !b)
		return true;
	if (!a || !b)
		return false;
	if (a == b)
		return true;
	if (a->type != b->type)
		return false;
	switch (a->type)
	{
	case XML_ELEMENT_NODE:
	case XML_ATTRIBUTE_NODE:
		if (!a->ns ^ !b->ns)
			return false;
		/* The following line is safe */
		if ((a->ns != b->ns) && xmlStrcmp(a->ns->href, b->ns->href))
			return false;
		if (xmlStrcmp(a->name, b->name))
			return false;
		if (a->type == XML_ELEMENT_NODE)
		{
			if (!xplCheckNodeListEquality(a->children, b->children))
				return false;
			return xplCheckPropListEquality(a->properties, b->properties);
		} else
			return xplCheckNodeListEquality(a->children, b->children);
	case XML_CDATA_SECTION_NODE:
	case XML_TEXT_NODE:
	case XML_COMMENT_NODE:
		return !xmlStrcmp(a->content, b->content);
	case XML_ENTITY_REF_NODE:
		if ((a->doc == b->doc) && !xmlStrcmp(a->name, b->name))
			return true;
		a = (xmlNodePtr) xmlGetDocEntity(a->doc, a->name);
		b = (xmlNodePtr) xmlGetDocEntity(b->doc, b->name);
		if (!a || !b)
			return false;
		return xplCheckNodeListEquality(a->children, b->children);
	case XML_PI_NODE:
		return !xmlStrcmp(a->name, b->name) && !xmlStrcmp(a->content, b->content);
	case XML_NAMESPACE_DECL:
		return !xmlStrcmp(((xmlNsPtr) a)->href, ((xmlNsPtr) b)->href);
	default: /* what else?.. */
		return false;
	}
}

bool xplCheckNodeListEquality(xmlNodePtr a, xmlNodePtr b)
{
	if (!a && !b)
		return true;
	if (!a || !b)
		return false;
	if ((a->type == b->type) && (b->type == XML_ATTRIBUTE_NODE))
		return xplCheckPropListEquality((xmlAttrPtr) a, (xmlAttrPtr) b);
	while (a && b)
	{
		if (!xplCheckNodeEquality(a, b))
			return false;
		a = a->next;
		b = b->next;
	}
	if (a || b)
		return false;
	return true;
}

bool xplCheckPropListEquality(xmlAttrPtr a, xmlAttrPtr b)
{
	/* the same attributes can be written in different order - so we imply sorting but don't require it */
	xmlAttrPtr prop_a, prop_b, cur_b;
	bool match;
	/* first check if there's the same number of props */
	prop_a = a, prop_b = b;
	while (prop_a && prop_b)
	{
		prop_a = prop_a->next;
		prop_b = prop_b->next;
	}
	if (prop_a || prop_b)
		return false;
	/* iterate through all props */
	prop_a = a, prop_b = b;
	while (prop_a)
	{
		cur_b = prop_b, match = 0;
		do 
		{
			if (xplCheckNodeEquality((xmlNodePtr) prop_a, (xmlNodePtr) cur_b))
			{
				match = 1;
				break;
			}
			cur_b = cur_b->next;
			if (!cur_b)
				cur_b = b;
		} while (cur_b != prop_b);
		if (!match)
			return false;
		prop_a = prop_a->next;
		prop_b = prop_b->next;
	}
	return true;
}

bool xplCheckNodeSetIdentity(xmlNodeSetPtr a, xmlNodeSetPtr b)
{
	size_t i, j, max;
	bool match;
	/* empty sets are most probably NOT identical: we have no idea what was requested by selectors */
	if (!a || !b)
		return false;
	if (a->nodeNr != b->nodeNr)
		return false;
	max = (size_t) a->nodeNr;
	for (i = 0; i < max; i++)
	{
		match = false, j = i;
		do {
			if (a->nodeTab[i] == b->nodeTab[j])
			{
				match = true;
				break;
			}
			if (++j >= max)
				j = 0;
		} while (j != i);
		if (!match)
			return false;
	}
	return true;
}

bool xplCheckNodeSetEquality(xmlNodeSetPtr a, xmlNodeSetPtr b)
{
	size_t i, j, max;
	bool match;
	/* empty sets are equal */
	if (!a && !b)
		return true;
	if (!a || !b)
		return false;
	if (a->nodeNr != b->nodeNr)
		return false;
	max = (size_t) a->nodeNr;
	for (i = 0; i < max; i++)
	{
		match = false, j = i;
		do {
			if (xplCheckNodeEquality(a->nodeTab[i], b->nodeTab[j]))
			{
				match = true;
				break;
			}
			if (++j >= max)
				j = 0;
		} while (j != i);
		if (!match)
			return false;
	}
	return true;
}

bool xplCompareXPathSelections(xmlXPathObjectPtr a, xmlXPathObjectPtr b, bool checkEquality)
{
	if (!a && !b)
		return checkEquality;
	if (!a || !b)
		return false;
	if (a->type != b->type)
		return false;
	switch (a->type)
	{
	case XPATH_BOOLEAN:
		return (a->boolval == b->boolval);
	case XPATH_NUMBER:
		return (a->floatval == b->floatval);
	case XPATH_STRING:
		return (!xmlStrcmp(a->stringval, b->stringval));
	case XPATH_NODESET:
		if (checkEquality)
			return xplCheckNodeSetEquality(a->nodesetval, b->nodesetval);
		else
			return xplCheckNodeSetIdentity(a->nodesetval, b->nodesetval);
	default:
		return false;
	/* TODO XPATH_POINT, XPATH_RANGE, XPATH_LOCATIONSET, XPATH_USER - do we ever encounter them? */
	}
}

xmlXPathObjectPtr xplSelectNodesWithCtxt(xmlXPathContextPtr ctxt, xmlNodePtr src, xmlChar *expr)
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
		XPL_FREE(ctxt->namespaces);
    return ret;
}

xmlAttrPtr xplCreateAttribute(xmlNodePtr dst, xplQName qname, xmlChar *value, bool allowReplace)
{
	xmlAttrPtr prev;
	xmlNsPtr ns;

	if (qname.ns)
		prev = xmlHasNsProp(dst, qname.ncname, qname.ns->href);
	else
		prev = xmlHasProp(dst, qname.ncname);
	if (prev)
	{
		if (!allowReplace)
			return prev;
		xmlFreeNodeList(prev->children);
		xplSetChildren((xmlNodePtr) prev, xmlNewDocText(dst->doc, value));
		return prev;
	}
	if (qname.ns)
	{
		ns = newReconciliedNs(dst->doc, dst, qname.ns);
		return xmlNewNsProp(dst, ns, qname.ncname, value);
	}
	return xmlNewProp(dst, qname.ncname, value);
}
