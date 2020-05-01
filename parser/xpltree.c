#include <libxpl/xpltree.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libxpl/abstraction/xpr.h>
#include <libxml/xmlsave.h>

xmlNodePtr findTail(xmlNodePtr cur)
{
	if (!cur)
		return NULL;
	while (cur->next)
		cur = cur->next;
	return cur;
}

bool isAncestor(xmlNodePtr maybeChild, xmlNodePtr maybeAncestor)
{
	if (maybeChild == maybeAncestor)
		return 0;
	if (!maybeChild || !maybeAncestor || !maybeChild->parent)
		return 0;
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

xmlNsPtr getResultingNs(xmlNodePtr parent, xmlNodePtr invoker, xmlChar *name)
{
	xmlNsPtr ret;
	/* TODO this is terribly wrong. Hrefs must be used */
	if ((ret = xmlSearchNs(parent->doc, parent, name)))
		return ret;
	if ((ret = xmlSearchNs(invoker->doc, invoker, name)))
		return xmlNewNs(parent, ret->href, ret->prefix);
	return NULL;
}

/* from libxml2 internals (tree.c) */
xmlChar* getPropValue(xmlAttrPtr prop)
{
	xmlChar *ret;

	if (!prop)
		return NULL;
	if (prop->type != XML_ATTRIBUTE_NODE)
		return NULL;
	if (!prop->children)
		return xmlStrdup(BAD_CAST "");
	if (!prop->children->next && ((prop->children->type == XML_TEXT_NODE) || (prop->children->type == XML_CDATA_SECTION_NODE)))
		return xmlStrdup(prop->children->content);
	return xmlNodeListGetString(prop->doc, prop->children, 1);
}

void unlinkProp(xmlAttrPtr cur)
{
	xmlAttrPtr tmp = cur->parent->properties;
	if (tmp == cur) {
		cur->parent->properties = cur->next;
		if (cur->next != NULL)
			cur->next->prev = NULL;
		return;
	}
	while (tmp != NULL) {
		if (tmp->next == cur) {
			tmp->next = cur->next;
			if (tmp->next != NULL)
				tmp->next->prev = tmp;
			return;
		}
		tmp = tmp->next;
	}
}

/* TODO change signature. this may fail (if ns is not found or tag name is invalid */
void assignAttribute(xmlNodePtr src, xmlNodePtr dst, xmlChar *name, xmlChar *value, bool allowReplace)
{
	xmlNsPtr ns = NULL;
	xmlChar *colon_pos, *tagname;
	xmlAttrPtr prev;

	colon_pos = (xmlChar*) xmlStrchr(name, ':');
	if (colon_pos)
	{
		*colon_pos = 0;
		ns = getResultingNs(dst, src, name);
		*colon_pos = ':';
		tagname = colon_pos + 1;
		if (ns)
			prev = xmlHasNsProp(dst, tagname, ns->href);
		else /* TODO fail here */
			prev = xmlHasProp(dst, tagname);
	} else {
		tagname = name;
		prev = xmlHasProp(dst, tagname);
	}
	if (prev)
	{
		if (!allowReplace)
			return;
		xmlFreeNodeList(prev->children);
		setChildren((xmlNodePtr) prev, xmlNewDocText(dst->doc, value));
	} else {
		if (ns)
			xmlNewNsProp(dst, ns, tagname, value);
		else
			xmlNewProp(dst, tagname, value);
	}
}

/* TODO distinguish out of memory condition from invalid name */
xmlNodePtr createElement(xmlNodePtr parent, xmlNodePtr invoker, xmlChar *name)
{
	xmlChar *tagname;
	xmlNsPtr ns;

	if ((tagname = BAD_CAST xmlStrchr(name, ':')))
	{
		*tagname = 0;
		ns = getResultingNs(parent, invoker, name);
		*tagname++ = ':';
	} else {
		tagname = name;
		ns = NULL;
	}
	if (xmlValidateName(tagname, 0))
		return NULL;
	return xmlNewDocNode(parent->doc, ns, tagname, NULL);
}

xmlNodePtr detachContent(xmlNodePtr el)
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

static xmlNodePtr setListParent(xmlNodePtr el, xmlNodePtr list)
{
	list->parent = el;
	while (list->next)
	{
		list = list->next;
		list->parent = el;
	}
	return list;
}

xmlNodePtr setChildren(xmlNodePtr el, xmlNodePtr list)
{
	if (!el)
		return NULL;
	el->children = list;
	if (!list)
	{
		el->last = NULL;
		return NULL;
	}
	return el->last = setListParent(el, list);
}

/* TODO all tree modifications must flatten adjacent text nodes */
xmlNodePtr appendChildren(xmlNodePtr el, xmlNodePtr list)
{
	if (!list)
		return NULL;
	if (!el)
		return findTail(list);
	if (el->children)
		return appendList(el->last? el->last: findTail(el->children), list);
	el->children = list;
	return el->last = setListParent(el, list);
}

xmlNodePtr appendList(xmlNodePtr el, xmlNodePtr list)
{
	xmlNodePtr tail;

	if (!list)
		return NULL;
	if (!el)
		return findTail(list);
	tail = setListParent(el->parent, list);
	tail->next = el->next;
	if (el->next)
		el->next->prev = tail;
	el->next = list;
	list->prev = el;
	if (el->parent && (el->parent->last == el))
		el->parent->last = tail;
	return tail;
}

xmlNodePtr prependList(xmlNodePtr el, xmlNodePtr list)
{
	xmlNodePtr tail;

	if (!list)
		return NULL;
	if (!el)
		return findTail(list);
	list->prev = el->prev;
	if (el->prev)
		el->prev->next = list;
	tail = setListParent(el->parent, list);
	el->prev = tail;
	tail->next = el;
	if (el->parent && (el->parent->children == el))
		el->parent->children = list;
	return tail;
}

xmlNodePtr replaceWithList(xmlNodePtr el, xmlNodePtr list)
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
	tail = setListParent(el->parent, list);
	if (el->next)
		el->next->prev = tail;
	tail->next = el->next;
	el->prev = el->next = el->parent = NULL;
	if (el->parent && el->parent->children == el)
		el->parent->children = list;
	if (el->parent && el->parent->last == el)
		el->parent->last = tail;
	return el;
}

xmlNodePtr cloneAttrAsText(xmlNodePtr cur, xmlNodePtr parent)
{
	xmlNodePtr ret;

	if (!cur)
		return NULL;
	if (cur->type == XML_ATTRIBUTE_NODE)
	{
		if (cur->children)
		{
			if (cur->children->next)
			{
				ret = xmlNewDocText(parent->doc, NULL);			
				ret->content = xmlNodeListGetString(parent->doc, cur->children, 1);
			} else
				ret = xmlNewDocText(parent->doc, cur->children->content);
		} else 
			ret = xmlNewDocText(parent->doc, BAD_CAST "");
	} else {
		ret = cloneNode(cur, parent, parent->doc);
	}
	return ret;
}

bool checkNodeListForText(xmlNodePtr start)
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

bool checkNodeSetForText(xmlNodeSetPtr s)
{
	size_t i;

	if (!s)
		return false;
	for (i = 0; i < (size_t) s->nodeNr; i++)
		if ((s->nodeTab[i]->type != XML_TEXT_NODE) 
			&& (s->nodeTab[i]->type != XML_ATTRIBUTE_NODE)
			&& (s->nodeTab[i]->type != XML_CDATA_SECTION_NODE)
		)
			return false;
	return true;
}

void markAncestorAxisForDeletion(xmlNodePtr bottom, xmlNodePtr top)
{
	if (!bottom || !top)
		return;
	while (1) 
	{
		bottom->type = (xmlElementType) ((int) bottom->type | XML_NODE_DELETION_REQUEST_FLAG);
		if (bottom == top)
			break;
		bottom = bottom->parent;
	} 
}

void markDOSAxisForDeletion(xmlNodePtr cur, int bitwiseAttribute, bool doMark)
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
		markDOSAxisForDeletion(cur, bitwiseAttribute, doMark);
		cur = cur->next;
	}
}

void deleteNeighbours(xmlNodePtr cur, xmlNodePtr boundary, bool markAncestorAxis)
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
			cur->type = (xmlElementType) ((int) cur->type | XML_NODE_DELETION_REQUEST_FLAG);
		cur = cur->parent;
	}
}

/* ����� libxml2 ����� �������, ������� ��������� �� ��������,
   �� ��� �������, ��� ��������� �� Ns ������ �������� �������������
   �� ��������� � ��������. ��� ���������, ����� ���� �������� NsDef
   ����� ����������� ���������� libxml2. �� ���� ������� ����������
   ����������� �� �������� ����� ���������� ����������� �������.
*/
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

/* ��������� �� ����������� ������ � ����������, 
   ������� ���������������� �� ������.
 */

static xmlChar* xmlStringText;
static xmlChar* xmlStringComment;

void initNamePointers()
{
	xmlNodePtr cur = xmlNewText(NULL);
	xmlStringText = BAD_CAST cur->name;
	xmlFree(cur);
	cur = xmlNewComment(NULL);
	xmlStringComment = BAD_CAST cur->name;
	xmlFree(cur);
}

xmlNsPtr newReconciliedNs(xmlDocPtr doc, xmlNodePtr tree, xmlNsPtr ns) 
{
    xmlNsPtr def;
    xmlChar prefix[50];
    int counter = 1;

    if (!tree)
		return NULL;
    if ((!ns) || (ns->type != XML_NAMESPACE_DECL)) 
		return NULL;
    /*
     * Search an existing namespace definition inherited.
     */
    def = xmlSearchNsByHref(doc, tree, ns->href);
    if (def)
        return(def);

    /*
     * Find a close prefix which is not already in use.
     * Let's strip namespace prefixes longer than 20 chars !
     */
    if (!ns->prefix)
		snprintf((char *) prefix, sizeof(prefix), "default");
    else
		snprintf((char *) prefix, sizeof(prefix), "%.20s", (char *)ns->prefix);

    def = xmlSearchNs(doc, tree, prefix);
    while (def) 
	{
        if (counter > 1000) 
			return NULL;
		if (!ns->prefix)
			snprintf((char *) prefix, sizeof(prefix), "default%d", counter++);
		else
			snprintf((char *) prefix, sizeof(prefix), "%.20s%d", (char *)ns->prefix, counter++);
		def = xmlSearchNs(doc, tree, prefix);
    }

    /*
     * OK, now we are ready to create a new one.
     */
    def = xmlNewNs(tree, ns->href, prefix);
    return def;
}


xmlAttrPtr clonePropInternal(xmlDocPtr doc, xmlNodePtr target, xmlAttrPtr cur, xmlNodePtr top_clone) 
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
			/*
			* Humm, we are copying an element whose namespace is defined
			* out of the new tree scope. Search it in the original tree
			* and add it at the top of the new tree
			*/
			ns = xmlSearchNs(cur->doc, cur->parent, cur->ns->prefix);
			if (ns) 
			{
				xmlNodePtr root = target;
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
		     * we have to find something appropriate here since
			 * we cant be sure, that the namespce we found is identified
			 * by the prefix
			 */
			if (xmlStrEqual(ns->href, cur->ns->href)) 
			{
				/* this is the nice case */
				ret->ns = ns;
			} else {
				/*
				* we are in trouble: we need a new reconcilied namespace.
				* This is expensive
				*/
				ret->ns = newReconciliedNs(target->doc, target, cur->ns);
			}
		}
    } else
        ret->ns = NULL;

    if (cur->children) 
	{
		xmlNodePtr tmp;
		ret->children = cloneNodeList(cur->children, (xmlNodePtr) ret, ret->doc);
		ret->last = NULL;
		tmp = ret->children;
		while (tmp) 
		{
		    if (!tmp->next)
				ret->last = tmp;
			tmp = tmp->next;
		}
    }
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
				xmlFree(id);
			}
		}
    }
    return(ret);
}

xmlAttrPtr clonePropListInner(xmlDocPtr doc, xmlNodePtr target, xmlAttrPtr cur, xmlNodePtr top_clone) 
{
    xmlAttrPtr ret = NULL;
    xmlAttrPtr p = NULL,q;

    while (cur) 
	{
        q = clonePropInternal(doc, target, cur, top_clone);
		if (!q)
			return(NULL);
		if (!p) 
		{
			ret = p = q;
		} else {
			p->next = q;
			q->prev = p;
			p = q;
		}
		cur = cur->next;
    }
    return(ret);
}

xmlNodePtr cloneNodeListInner(xmlNodePtr node, xmlNodePtr parent, xmlDocPtr doc, xmlNodePtr top_clone);

xmlNodePtr cloneNodeInner(xmlNodePtr node, xmlNodePtr parent, xmlDocPtr doc, xmlNodePtr top_clone)
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
    ret = (xmlNodePtr) xmlMalloc(sizeof(xmlNode));
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
	/* ���� ������ �������� ������ ��� ������ � libxslt */
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
			ret->name = xmlStrdup(node->name);
    }
    if ((node->type != XML_ELEMENT_NODE) &&
		(node->content != NULL) &&
		(node->type != XML_ENTITY_REF_NODE) &&
		(node->type != XML_XINCLUDE_END) &&
		(node->type != XML_XINCLUDE_START)) 
	{
		ret->content = xmlStrdup(node->content);
    } else {
      if (node->type == XML_ELEMENT_NODE)
        ret->line = node->line;
    }
    if (parent != NULL && ret != top_clone) 
	{
		xmlNodePtr tmp;

		/* ��� �� �� ���������� */
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

    /* ��� �� �� ���������� */
	/*
    if ((!parent) && ((__xmlRegisterCallbacks) && (xmlRegisterNodeDefaultValue)))
		xmlRegisterNodeDefaultValue((xmlNodePtr)ret);
	 */
	if (ret == top_clone)
		ret->parent = NULL;
    return(ret);
}

xmlNodePtr cloneNodeListInner(xmlNodePtr node, xmlNodePtr parent, xmlDocPtr doc, xmlNodePtr top_clone)
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

xmlNodePtr cloneNode(xmlNodePtr node, xmlNodePtr parent, xmlDocPtr doc)
{
	return cloneNodeInner(node, parent, doc, NULL);
}

xmlNodePtr cloneNodeList(xmlNodePtr node, xmlNodePtr parent, xmlDocPtr doc)
{
	return cloneNodeListInner(node, parent, doc, NULL);
}

void addNsDef(xmlNodePtr cur, xmlNsPtr def)
{
	xmlNsPtr last_ns = cur->nsDef;
	if (!last_ns)
		cur->nsDef = def;
	else {
		while (last_ns->next)
			last_ns = last_ns->next;
		last_ns->next = def;
	}
}

void downshiftNodeNsDef(xmlNodePtr cur, xmlNsPtr ns_list)
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
					addNsDef(cur, ns_to_shift);
				}
				break;
			}
			ns_cur = ns_cur->next;
		}
	}
	/* ������ �� ���� ��������� */
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
						addNsDef(cur, ns_to_shift);
					}
					break;
				}
				ns_cur = ns_cur->next;
			}
		}
		prop = prop->next;
	}
	if (cur->children)
		downshiftNodeListNsDef(cur->children, ns_list);
}

void downshiftNodeListNsDef(xmlNodePtr cur, xmlNsPtr ns_list)
{
	if (!ns_list)
		return;
	while (cur)
	{
		downshiftNodeNsDef(cur, ns_list);
		cur = cur->next;
	}
}

typedef struct _NsReplacementContext
{
	xmlNsPtr *old_ns;
	xmlNsPtr *new_ns;
	size_t count;
	size_t cache_size;
} NsReplacementContext;
typedef NsReplacementContext *NsReplacementContextPtr;

static bool nsIsIndep(xmlNsPtr ns, xmlNodePtr top, xmlNodePtr elem)
{
	xmlNsPtr cur_ns;

	do
	{
		cur_ns = elem->nsDef;
		while(cur_ns)
		{
			if (cur_ns == ns)
				return true;
			cur_ns = cur_ns->next;
		}
		elem = elem->parent;
	} while (elem != top->parent);
	return false;
}

static xmlNsPtr getIndepNs(NsReplacementContextPtr ctxt, xmlNodePtr top, xmlNodePtr elem, xmlNsPtr ns)
{
	size_t i;
	xmlNsPtr ret;

	for (i = 0; i < ctxt->count; i++)
		if (ctxt->old_ns[i] == ns)
			return ctxt->new_ns[i];
	if (++ctxt->count > ctxt->cache_size)
	{
		ctxt->cache_size *= 2;
		ctxt->old_ns = (xmlNsPtr*) xmlRealloc(ctxt->old_ns, ctxt->cache_size);
		ctxt->new_ns = (xmlNsPtr*) xmlRealloc(ctxt->new_ns, ctxt->cache_size);
	}
	if (nsIsIndep(ns, top, elem))
		ret = ns;
	else 
		ret = xmlNewNs(top, ns->href, ns->prefix);
	ctxt->old_ns[ctxt->count-1] = ns;
	ctxt->new_ns[ctxt->count-1] = ret;
	return ret;
}

static void makeNsIndepTreeInner(NsReplacementContextPtr ctxt, xmlNodePtr top, xmlNodePtr cur)
{
	xmlAttrPtr attr;
	if (cur->type != XML_ELEMENT_NODE)
		return;
	if (cur->ns)
		cur->ns = getIndepNs(ctxt, top, cur, cur->ns);
	attr = cur->properties;
	while(attr)
	{
		if (attr->ns)
			attr->ns = getIndepNs(ctxt, top, cur, attr->ns);
		attr = attr->next;
	}
	cur = cur->children;
	while(cur)
	{
		makeNsIndepTreeInner(ctxt, top, cur);
		cur = cur->next;
	}
}

void makeNsIndepTree(xmlNodePtr top)
{
	NsReplacementContext ctxt;

	if (top->type != XML_ELEMENT_NODE)
		return;
	ctxt.count = 0;
	ctxt.cache_size = 16;
	ctxt.old_ns = (xmlNsPtr*) xmlMalloc(sizeof(xmlNsPtr)*ctxt.cache_size);
	ctxt.new_ns = (xmlNsPtr*) xmlMalloc(sizeof(xmlNsPtr)*ctxt.cache_size);
	makeNsIndepTreeInner(&ctxt, top, top);
	xmlFree(ctxt.old_ns);
	xmlFree(ctxt.new_ns);
}

/* ������ �������� ������ - �������� ���������� nsdef. */

/* �������� ������������ ������������ ���. �������, ��� carrier - 
   �������, ������� ��� ����������� (�� NULL!)
 */
static xmlNsPtr getIrredundantNsByAncestor(xmlNsPtr ns, xmlNodePtr carrier)
{
	xmlNsPtr cur_ns, ret = ns, six;
	while ((carrier = carrier->parent))
	{
		cur_ns = carrier->nsDef;
		while (cur_ns)
		{
			if (!xmlStrcmp(cur_ns->href, ret->href))
			{
				/* ������ ���������� ����������� ����� �����, ����� �� ������������ �������� */
				if (carrier->nsDef == ret)
					carrier->nsDef = six->next;
				else {
					six = carrier->nsDef;
					while (six->next != ret)
						six = six->next;
					six->next = ret->next;
				}
				xmlFreeNs(ret);
				ret = cur_ns; /* ��������� */
				break;
			}
		}
	}
	return ns;
}

static xmlNsPtr getIrredundantNs(NsReplacementContextPtr ctxt, xmlNodePtr carrier)
{
	size_t i;
	xmlNsPtr ret;

	for (i = 0; i < ctxt->count; i++)
		if (ctxt->old_ns[i] == carrier->ns)
			return ctxt->new_ns[i];
	if (++ctxt->count > ctxt->cache_size)
	{
		ctxt->cache_size *= 2;
		ctxt->old_ns = (xmlNsPtr*) xmlRealloc(ctxt->old_ns, ctxt->cache_size);
		ctxt->new_ns = (xmlNsPtr*) xmlRealloc(ctxt->new_ns, ctxt->cache_size);
	}
	ret = getIrredundantNsByAncestor(carrier->ns, carrier);
	ctxt->old_ns[ctxt->count] = carrier->ns;
	ctxt->new_ns[ctxt->count] = ret;
	return ret;
}

static void replaceRedundantNamespacesInner(NsReplacementContextPtr ctxt, xmlNodePtr cur)
{
	xmlAttrPtr attr;
	if (cur->type != XML_ELEMENT_NODE)
		return;
	if (cur->ns)
		cur->ns = getIrredundantNs(ctxt, cur);
	attr = cur->properties;
	while(attr)
	{
		if (attr->ns)
			attr->ns = getIrredundantNs(ctxt, (xmlNodePtr) attr);
		attr = attr->next;
	}
	cur = cur->children;
	while(cur)
	{
		replaceRedundantNamespacesInner(ctxt, cur);
		cur = cur->next;
	}
}

void replaceRedundantNamespaces(xmlNodePtr top)
{
	NsReplacementContext ctxt;

	if (top->type != XML_ELEMENT_NODE)
		return;
	ctxt.count = 0;
	ctxt.cache_size = 16;
	ctxt.old_ns = (xmlNsPtr*) xmlMalloc(sizeof(xmlNsPtr)*ctxt.cache_size);
	ctxt.new_ns = (xmlNsPtr*) xmlMalloc(sizeof(xmlNsPtr)*ctxt.cache_size);
	replaceRedundantNamespacesInner(&ctxt, top);
	xmlFree(ctxt.old_ns);
	xmlFree(ctxt.new_ns);
}

/* XPath extensions */
/* not now: some heavy funcs in libxml2 are static */
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
#endif
}

/* serialization */

void safeSerializeContent(FILE *fp, xmlChar* content)
{
#define BUF_SIZE 1024
	xmlChar buf[BUF_SIZE], *buf_ptr;

	if (!fp || !content)
		return;
	buf_ptr = buf;
	while (*content)
	{
		switch (*content++)
		{
		case '<':
			*buf_ptr++ = '&';
			*buf_ptr++ = 'l';
			*buf_ptr++ = 't';
			*buf_ptr++ = ';';
			break;
		case '>':
			*buf_ptr++ = '&';
			*buf_ptr++ = 'g';
			*buf_ptr++ = 't';
			*buf_ptr++ = ';';
			break;
		case '&':
			*buf_ptr++ = '&';
			*buf_ptr++ = 'a';
			*buf_ptr++ = 'm';
			*buf_ptr++ = 'p';
			*buf_ptr++ = ';';
			break;
		case '"':
			*buf_ptr++ = '&';
			*buf_ptr++ = 'q';
			*buf_ptr++ = 'u';
			*buf_ptr++ = 'o';
			*buf_ptr++ = 't';
			*buf_ptr++ = ';';
			break;
		default:
			*buf_ptr++ = *content;
		}
		if ((buf_ptr - buf) >= (BUF_SIZE - 8))
		{
			*buf_ptr = 0;
			fprintf(fp, "%s", buf);
			buf_ptr = buf;
		}
	}
	if (buf_ptr > buf)
	{
		*buf_ptr = 0;
		fprintf(fp, "%s", buf);
	}
#undef BUF_SIZE
}

void safeSerializeNode(FILE *fp, xmlNodePtr node, int indent)
{
	xmlNsPtr nsdefs;
	int i;

	if (!fp || !node)
		return;
	switch(node->type)
	{
	case XML_ELEMENT_NODE:
		/* start element */
		for (i = 0; i < indent; i++)
			fprintf(fp, " ");
		if (node->ns)
			fprintf(fp, "<%s:%s", node->ns->prefix, node->name);
		else
			fprintf(fp, "<%s", node->name);
		/* cycle through nsdefs */
		nsdefs = node->nsDef;
		while (nsdefs)
		{
			if (nsdefs->prefix)
				fprintf(fp, " xmlns:%s=\"%s\"", nsdefs->prefix, nsdefs->href);
			else
				fprintf(fp, " xmlns=\"%s\"", nsdefs->href);
			nsdefs = nsdefs->next;
		}
		/* cycle through attributes */
		safeSerializeNodeList(fp, (xmlNodePtr) node->properties, indent);
		fprintf(fp, ">");
		/* serialize children */
		safeSerializeNodeList(fp, node->children, indent + 2);
		/* end element */
		for (i = 0; i < indent; i++)
			fprintf(fp, " ");
		if (node->ns)
			fprintf(fp, "</%s:%s>\n", node->ns->prefix, node->name);
		else
			fprintf(fp, "</%s>\n", node->name);
		break;
	case XML_TEXT_NODE:
		if (node->content)
			safeSerializeContent(fp, node->content);
		break;
	case XML_ATTRIBUTE_NODE:
		if (node->ns)
			fprintf(fp, "%s:%s=\"", node->ns->prefix, node->name);
		else
			fprintf(fp, "%s=\"", node->name);
		safeSerializeNodeList(fp, node->children, indent);
		fprintf(fp, "\"");
		break;
	case XML_ENTITY_NODE:
		/* ToDo */
		break;
	case XML_ENTITY_REF_NODE:
		/* ToDo */
		break;
	default:
		return;
	}
}

void safeSerializeNodeList(FILE *fp, xmlNodePtr list, int indent)
{
	if (!fp)
		return;
	while (list)
	{
		safeSerializeNode(fp, list, indent);
		list = list->next;
	}
}

void safeSerializeDocument(char *filename, xmlDocPtr doc)
{
	FILE *fp = fopen(filename, "w");
	if (!fp)
		return;
	fprintf(fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	safeSerializeNode(fp, doc->children, 0);
	fclose(fp);
}

/* document saving */

static int xml_save_write_cb(void *context, const char *buffer, int len)
{
	if (!len)
		return 0;
	return write((int) (size_t) context, buffer, len);
}

static int xml_save_close_cb(void *context)
{
	close((int) (size_t) context);
	return 0;
}

bool saveXmlDocToFile(xmlDocPtr doc, xmlChar *filename, char *encoding, int options)
{
	int fh, ret;
	xmlSaveCtxtPtr save_ctxt;

	fh = xprSOpen(filename, O_CREAT | O_TRUNC | O_RDWR | O_BINARY, 0, S_IREAD | S_IWRITE);
	if (fh == -1)
		return false;
	save_ctxt = xmlSaveToIO(xml_save_write_cb, xml_save_close_cb, (void*) (size_t) fh, encoding, options);
	if (!save_ctxt)
		return false;
	xmlSaveSetEscape(save_ctxt, NULL);
	xmlSaveSetAttrEscape(save_ctxt, NULL);
	xmlSaveDoc(save_ctxt, doc);
	xmlSaveClose(save_ctxt);
	return true;
}


/* serialization */

xmlChar* serializeNodeList(xmlNodePtr cur) 
{
	xmlBufferPtr buf;
	xmlSaveCtxtPtr save_ctxt;
	xmlChar* ret;
	
	if (!cur) 
		return NULL;
	buf = xmlBufferCreate();
	if (!buf)
		return NULL;
	save_ctxt = xmlSaveToBuffer(buf, NULL, 0);
	if (!save_ctxt)
	{
		xmlBufferFree(buf);
		return NULL;
	}
	xmlSaveSetEscape(save_ctxt, NULL);
	while (cur != NULL) 
	{
		xmlSaveTree(save_ctxt, cur);
		cur = cur->next;
	}
	xmlSaveFlush(save_ctxt);
	ret = buf->content;
	buf->content = NULL;
	xmlSaveClose(save_ctxt);
	xmlBufferFree(buf);
	return ret;
}

xmlChar* serializeNodeSet(xmlNodeSetPtr set) 
{
	size_t i;
	xmlNodePtr cur;
	xmlBufferPtr buf;
	xmlSaveCtxtPtr save_ctxt;
	xmlChar *ret;

	if (!set) 
		return NULL;
	buf = xmlBufferCreate();
	if (!buf)
		return NULL;
	save_ctxt = xmlSaveToBuffer(buf, NULL, 0);
	if (!save_ctxt)
	{
		xmlBufferFree(buf);
		return NULL;
	}
	xmlSaveSetEscape(save_ctxt, NULL);
	for (i = 0; i < (size_t) set->nodeNr; i++)
	{
		cur = set->nodeTab[i];
		xmlSaveTree(save_ctxt, cur);
		cur = cur->next;
	}
	xmlSaveFlush(save_ctxt);
	ret = buf->content;
	buf->content = NULL;
	xmlSaveClose(save_ctxt);
	xmlBufferFree(buf);
	return ret;
}

bool checkNodeEquality(xmlNodePtr a, xmlNodePtr b)
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
			if (!checkNodeListEquality(a->children, b->children))
				return false;
			return checkPropListEquality(a->properties, b->properties);
		} else
			return checkNodeListEquality(a->children, b->children);
	case XML_CDATA_SECTION_NODE:
	case XML_TEXT_NODE:
	case XML_COMMENT_NODE:
		return !xmlStrcmp(a->content, b->content);
	case XML_ENTITY_REF_NODE:
		if ((a->doc == b->doc) && !xmlStrcmp(a->name, b->name))
			return true;
		a = (xmlNodePtr) xmlGetDocEntity(a->doc, a->name);
		b = (xmlNodePtr) xmlGetDocEntity(b->doc, b->name);
		/* ���� a == b == null, ���� �� �����: ������ �� �������������� �������� ����� ����� ���� ������ */
		if (!a || !b)
			return false;
		return checkNodeListEquality(a->children, b->children);
	case XML_PI_NODE:
		return !xmlStrcmp(a->name, b->name) && !xmlStrcmp(a->content, b->content);
	case XML_NAMESPACE_DECL:
		return !xmlStrcmp(((xmlNsPtr) a)->href, ((xmlNsPtr) b)->href);
	default: /* what else?.. */
		return false;
	}
}

bool checkNodeListEquality(xmlNodePtr a, xmlNodePtr b)
{
	if (!a && !b)
		return true;
	if (!a || !b)
		return false;
	if ((a->type == b->type) && (b->type == XML_ATTRIBUTE_NODE))
		return checkPropListEquality((xmlAttrPtr) a, (xmlAttrPtr) b);
	while (a && b)
	{
		if (!checkNodeEquality(a, b))
			return false;
		a = a->next;
		b = b->next;
	}
	if (a || b)
		return false;
	return true;
}

bool checkPropListEquality(xmlAttrPtr a, xmlAttrPtr b)
{
	/* ��������� ����������� � ��������� �� ������� ����������, �������� �������� "��������������, �� �� ��������� ����������" */
	xmlAttrPtr prop_a, prop_b, cur_b;
	bool match;
	/* ������� ��-���, ��� ����������� ��� �������, ��� ��������� ������ ��������� */
	prop_a = a, prop_b = b;
	while (prop_a && prop_b)
	{
		prop_a = prop_a->next;
		prop_b = prop_b->next;
	}
	if (prop_a || prop_b)
		return false;
	/* ���������� �����, �������� ������ */
	prop_a = a, prop_b = b;
	while (prop_a)
	{
		cur_b = prop_b, match = 0;
		do 
		{
			if (checkNodeEquality((xmlNodePtr) prop_a, (xmlNodePtr) cur_b))
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

bool checkNodeSetIdentity(xmlNodeSetPtr a, xmlNodeSetPtr b)
{
	size_t i, j, max;
	bool match;
	/* �������, ��������, �� �����: ��������� ����� ������� ������ ������, ������� �� ��� ���-������. */
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

bool checkNodeSetEquality(xmlNodeSetPtr a, xmlNodeSetPtr b)
{
	size_t i, j, max;
	bool match;
	/* ������� ����� */
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
			if (checkNodeEquality(a->nodeTab[i], b->nodeTab[j]))
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

bool compareXPathSelections(xmlXPathObjectPtr a, xmlXPathObjectPtr b, bool checkEquality)
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
			return checkNodeSetEquality(a->nodesetval, b->nodesetval);
		else
			return checkNodeSetIdentity(a->nodesetval, b->nodesetval);
	default:
		return false;
	}
}
