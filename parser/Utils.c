#include "Utils.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
/* extra libxml2 headers */
#include <libxml/chvalid.h>
#include <libxml/xpathInternals.h>
#include <libxml/uri.h>
#ifdef _USE_LIBIDN
# include <idna.h>
# include <idn-free.h>
#endif

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
	el->children = NULL;
	el->last = NULL;
	return item;
}

xmlNodePtr setChildren(xmlNodePtr el, xmlNodePtr list)
{
	xmlNodePtr n;
	if (!el)
		return NULL;
	el->children = list;
	if (!list)
	{
		el->last = NULL;
		return NULL;
	}
	n = list;
	n->parent = el;
	while (n->next)
	{
		n = n->next;
		n->parent = el;
	}
	el->last = n;
	return n;
}

xmlNodePtr appendChildren(xmlNodePtr el, xmlNodePtr list)
{
	if (!list)
		return NULL;
	if (!el)
		return findTail(list);
	if (el->children)
		return appendList(el->last? el->last: findTail(el->children), list);
	el->children = list;
	while (list->next)
	{
		list->parent = el;
		list = list->next;
	}
	list->parent = el;
	el->last = list;
	return list;
}

xmlNodePtr appendList(xmlNodePtr el, xmlNodePtr list)
{
	xmlNodePtr tail;

	if (!list)
		return NULL; /* empty list, nothing to do */
	list->parent = el->parent;
	tail = list;
	while (tail->next)
	{
		tail = tail->next;
		tail->parent = el->parent;
	}
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
	xmlNodePtr n, prev;

	if (!list || !el)
		return NULL;
	n = list;
	n->parent = el->parent;
	while (n->next)
	{
		n->next->parent = el->parent;
		n = n->next;
	}
	prev = el->prev;
	el->prev = n;
	n->next = el;
	list->prev = prev;
	if (prev)
		prev->next = list;
	if (el->parent && (el->parent->children == el))
		el->parent->children = list;
	return n;
}

xmlNodePtr replaceWithList(xmlNodePtr el, xmlNodePtr list)
{
	xmlNodePtr prev, next;

	if (!el)
		return NULL;
	if (!list)
	{
		xmlUnlinkNode(el);
		return el;
	}
	prev = el->prev;
	/* list = textMergeHead(prev, list);*/
	if (prev)
		prev->next = list;
	list->prev = prev;
	list->parent = el->parent;
	if (el->parent && el->parent->children == el)
		el->parent->children = list;
	while (list->next)
	{
		list->next->parent = el->parent;
		list = list->next;
	}
	next = el->next;
	if (next)
	//{
		next->prev = list;
	//	list = textMergeHead(list, next);
	//} else
		list->next = next;
	if (el->parent && el->parent->last == el)
		el->parent->last = list;
	el->prev = el->next = el->parent = NULL;
	return el;
}

xmlChar* strTrim(xmlChar* str)
{
	size_t out_size;
	xmlChar *start, *end, *out;

	start = str;
	end = str + xmlStrlen(str) - 1;
	while (start <= end)
	{
		if (xmlIsBlank_ch(*start)) 
			start++;
		else break;
	}
	while (start < end)
	{
		if (xmlIsBlank_ch(*end))
			end--;
		else break;
	}
	if (start > end)
		return NULL;
	out_size = end - start + 1;
	out = (xmlChar*) xmlMalloc(out_size + 1);
	strncpy((char*) out, (const char*) start, out_size + 1);
	return out;
}

bool strNonblank(xmlChar* str)
{
	if (!str)
		return false;
	while (*str)
	{
		if (!xmlIsBlank_ch(*str))
			return true;
		str++;
	}
	return false;
}

bool isNumber(xmlChar *str)
{
	xmlChar *p;
	bool dot = false;

	if (!str)
		return false;
	p = str;
	while (*p)
	{
		if (isdigit(*p))
			NOOP();
		else if ((*p == '.') && !dot)
			dot = true;
		else
			return false;
		p++;
	}
	return true;
}

xmlChar* getLastLibxmlError()
{
	xmlChar *error, *encError;
	xmlErrorPtr err;
	size_t max_err_len;
	
	err = xmlGetLastError();
	if (!err)
		return xmlStrdup(BAD_CAST "unknown error");
	max_err_len = (err->message?strlen(err->message):0) + (err->file?strlen(err->file):0) + 127;
	if (err->str1) max_err_len += strlen(err->str1);
	if (err->str2) max_err_len += strlen(err->str2);
	if (err->str3) max_err_len += strlen(err->str3);
	error = (xmlChar*) xmlMalloc(max_err_len + 1); 
	if (!error)
		return NULL;
	snprintf((char*) error, max_err_len, "file %s, %d:%d, problem: %s, extra info [%s, %s, %s]", err->file, err->line, err->int2, err->message, err->str1, err->str2, err->str3);
	encError = xmlEncodeSpecialChars(NULL, error);
	xmlFree(error);
	return encError;
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


int isAncestor(xmlNodePtr cur, xmlNodePtr test)
{
	if (test == cur)
		return 0;
	if (!cur || !test || !cur->parent)
		return 0;
	while (cur)
	{
		if (cur->parent == test)
			return 1;
		if (cur == test->parent)
			return 0; /* speedup?.. */
		cur = cur->parent;
	}
	return 0;
}

xmlNodePtr findTail(xmlNodePtr cur)
{
	if (!cur)
		return NULL;
	while (cur->next)
		cur = cur->next;
	return cur;
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

static xmlChar hex_digits[] = "0123456789ABCDEF";

xmlChar* bufferToHex(void* buf, size_t len, bool prefix)
{
	xmlChar *ret, *ret_start;
	size_t i;

	ret = ret_start = (xmlChar*) xmlMalloc(len*2 + (prefix? 3: 1));
	if (!ret)
		return NULL;
	if (prefix)
	{
		*ret++ = '0';
		*ret++ = 'x';
	}
	for (i = 0; i < len; i++)
	{
		*ret++ = hex_digits[((xmlChar*) buf)[i] >> 4];
		*ret++ = hex_digits[((xmlChar*) buf)[i] & 0x0F];
	}
	*ret = 0;
	return ret_start;
}

/* �� libxml2 (tree.c) */
xmlChar* getPropValue(xmlAttrPtr prop)
{
	xmlChar *ret;

	if (!prop)
		return NULL;
	if (prop->type == XML_ATTRIBUTE_NODE) 
	{
		if (prop->children) 
		{
			if (!prop->children->next && ((prop->children->type == XML_TEXT_NODE) || (prop->children->type == XML_CDATA_SECTION_NODE)))
			{
				return xmlStrdup(prop->children->content);
			} else {
				ret = xmlNodeListGetString(prop->doc, prop->children, 1);
				if (ret)
					return ret;
			}
		}
		return xmlStrdup(BAD_CAST "");
	} 
	return NULL;
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

/* ���������� ���������� � ������ ������� �������� �������� �������������������
 * ����� ������ ������ ������������� � ������ ������������� ������������ ��-�� ������������� 
 * ���������� ������� ���, ��� �������� �������� ����������� lookahead. ������������� ������� 
 * �� ������ lookahead = �������� ��������� ���������� ���.
 * ������� � state machine, ��� ��������� ���������, �� �������. */
typedef enum _utf8CheckStep
{
	U8CS_DECISION = 0,
	U8CS_2_1,
	U8CS_3_1,
	U8CS_3_2,
	U8CS_4_1,
	U8CS_4_2,
	U8CS_4_3
} utf8CheckStep;

typedef struct _utf8CheckState
{
	uint8_t mask;
	uint8_t result;
	utf8CheckStep next;
} utf8CheckState, *utf8CheckStatePtr;

static utf8CheckState U8DecisionList[] = 
{
	{ 0x80, 0x00, U8CS_DECISION },
	{ 0xE0, 0xC0, U8CS_2_1 },
	{ 0xF0, 0xE0, U8CS_3_1 },
	{ 0xF8, 0xF0, U8CS_4_1 }
};
#define U8_DECISION_LIST_SIZE (sizeof(U8DecisionList)/sizeof(U8DecisionList[0]))

static utf8CheckState U8TransitionList[] = 
{
	{ 0xFF, 0x00, U8CS_DECISION },	/* �������� */
	{ 0xC0, 0x80, U8CS_DECISION },	/* U8CS_2_1 */
	{ 0xC0, 0x80, U8CS_3_2 },		/* U8CS_3_1 */
	{ 0xC0, 0x80, U8CS_DECISION },	/* U8CS_3_2 */
	{ 0xC0, 0x80, U8CS_4_2 },		/* U8CS_4_1 */
	{ 0xC0, 0x80, U8CS_4_3 },		/* U8CS_4_2 */
	{ 0xC0, 0x80, U8CS_DECISION }	/* U8CS_4_3 */
};

bool isValidUtf8Sample(xmlChar *s, size_t len, bool isCompleteString)
{
	xmlChar *end = s + len, *p = s;
	utf8CheckStep step = U8CS_DECISION;
	int i;

	while (p < end)
	{
		if (step == U8CS_DECISION) /* ��������� �� ������ ������� */
		{
			for (i = 0; i < U8_DECISION_LIST_SIZE; i++)
				if ((*p & U8DecisionList[i].mask) == U8DecisionList[i].result)
				{
					step = U8DecisionList[i].next;
					break;
				}
			if (i == U8_DECISION_LIST_SIZE) /* "��������, ������ �� ������� ��������� ��� ���" */
				return false;
		} else { /* ������� */
			if ((*p & U8TransitionList[step].mask) == U8TransitionList[step].result)
				step = U8TransitionList[step].next;
			else
				return false;
		}
		p++;
	}
	if (isCompleteString)
		return (step == U8CS_DECISION);
	return true;
}

#if 0
bool isValidUtf8Sample(xmlChar *s, size_t len)
{
	size_t i = 0;
	while (i < (len-4)) /* ����� ������ �� ������ ����� ������, � ��� ������� �� ����� ���� �������� �� ������� ������� */
	{
		if (!(s[i] & 0x80))
		{
			i++;
			continue; /* 0xxxxxxx */
		}
		if (((s[i] & 0xE0) == 0xC0) && ((s[i+1] & 0xC0) == 0x80))
		{
			i += 2;
			continue; /* 110xxxxx 10xxxxxx */
		}
		if (((s[i] & 0xF0) == 0xE0) && ((s[i+1] & 0xC0) == 0x80) && ((s[i+2] & 0xC0) == 0x80))
		{
			i += 3;
			continue; /* 1110xxxx 10xxxxxx 10xxxxxx */
		}
		if (((s[i] & 0xF8) == 0xF0) && ((s[i+1] & 0xC0) == 0x80) && ((s[i+2] & 0xC0) == 0x80) && ((s[i+3] & 0xC0) == 0x80)) 
		{
			i += 4;
			continue; /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
		}
		return false;
	}
	return true;
}
#endif 

int detectEncoding(char* str, size_t sampleLen)
{
	size_t small_count, caps_count;
	size_t a0af_count, f0ff_count;
	size_t i;
	size_t real_sample_len; /* �� ��������� � ���������� ������ �������� ASCII */
	unsigned char c;

	/* �� �� ��������� ����� ������: � ������ UTF-16 ������������ xmlStrlen ������������. */
	/* ������ ����. ���� ���� - �� ��� ������/���� ���������� � ��������� UTF-16 */
	for (i = 0; i < sampleLen; i++)
	{
		if (!str[i])
		{
			if (i % 2)
				return DETECTED_ENC_UTF16LE;
			else
				return DETECTED_ENC_UTF16BE;
		}
	}
	/* �������� �� utf-8 */
	if (isValidUtf8Sample(BAD_CAST str, sampleLen, false))
		return DETECTED_ENC_UTF8;
	/* ������ ������. ���� �� ������� �������� ���� ������, ��� �� ������, ��������� ���-8 */
	small_count = caps_count = 0;
	for (i = 0; i < sampleLen; i++)
	{
		c = (unsigned char) str[i];
		if ((c == 0xB8) || (c >= 0xE0))
			small_count++;
		else if ((c == 0xA8) || ((c >= 0xC0) && (c <= 0xDF)))
			caps_count++;
	}
	if ((caps_count > small_count) && small_count) /* ���� ����� ��� ���� �� ������� �������� */
		return DETECTED_ENC_KOI8;
	/* �������� �� ��������� �������������, �������������� � ��������� */
	a0af_count = f0ff_count = real_sample_len = 0;
    for (i = 0; i < sampleLen; i++)
    {
		c = (unsigned char) str[i];
		if ((c >= 0xA0) && (c <= 0xAF)) a0af_count++;
        else if (c >= 0xF0) f0ff_count++;
		if (c & 0x80) real_sample_len++;
    }
    if ((((float) a0af_count) / real_sample_len < 0.33985) && (((float) f0ff_count) / real_sample_len > 0.15105))
		return DETECTED_ENC_1251;
	else if (a0af_count || f0ff_count)
		return DETECTED_ENC_866;
	return DETECTED_ENC_UTF8; /* last resort */
}

/* ���� ������� ����� ������ ��� ����� �������� �������, ��� �����.
   ������ - ����-�� ������� ������. ������� ���������� ������� ����.
 */
#if 0
int iconv_string (const char* tocode, const char* fromcode,
                  const char* start, const char* end,
                  char** resultp, size_t* lengthp)
{
/* ������ ���������������� ��� Bruno Haible */
#define TMP_BUF_SIZE 4096
	int is_utf16, saved_errno;
	size_t length, size, count = 0, insize, outsize, res;
	char *result, *outptr;
	iconv_t cd;
	char tmpbuf[TMP_BUF_SIZE];
	const char *inptr;

	if (!strcmp(tocode, fromcode)) /* ����������� */
	{
		size = end - start;
		*resultp = (char*) (*resultp == NULL ? xmlMalloc(size+2) : xmlRealloc(*resultp, size+2));
		memcpy(*resultp, start, size);
		*resultp[size] = *resultp[size+1] = 0; /* �� utf-16 */
		return 0;
	}
	is_utf16  = (strstr(tocode, "utf-16") != 0);
	cd = iconv_open(tocode,fromcode);
	if (cd == (iconv_t) (-1)) 
		return -1;
	/* Determine the length we need. */
	inptr = start;
	insize = end - start;
	while (insize > 0) 
	{
		outptr = tmpbuf;
		outsize = TMP_BUF_SIZE;
		res = iconv(cd, &inptr, &insize, &outptr, &outsize);
		if (res == (size_t)(-1) && errno != E2BIG) 
		{
			if (errno == EINVAL)
				break;
			else {
				saved_errno = errno;
				iconv_close(cd);
				errno = saved_errno;
				return -1;
			}
		}
		count += (outptr - &tmpbuf[0]);
	}
	outptr = tmpbuf;
	outsize = TMP_BUF_SIZE;
	res = iconv(cd, NULL, NULL, &outptr, &outsize);
	if (res == (size_t) (-1)) 
	{
		saved_errno = errno;
		iconv_close(cd);
		errno = saved_errno;
		return -1;
	}
	count += (outptr - &tmpbuf[0]);
    length = count;

	if (lengthp)
		*lengthp = length;
	if (!resultp) 
	{
		iconv_close(cd);
		return 0;
	}
	result = (char*) (*resultp == NULL ? xmlMalloc(length+1+is_utf16) : xmlRealloc(*resultp, length+1+is_utf16));
	*resultp = result;
	if (!length) 
	{
		iconv_close(cd);
		return 0;
	}
	if (!result) 
	{
		iconv_close(cd);
		errno = ENOMEM;
		return -1;
	}
	iconv(cd, NULL, NULL, NULL, NULL); /* return to the initial state */
	/* Do the conversion for real. */
	inptr = start;
    insize = end - start;
    outptr = result;
    outsize = length;
    while (insize > 0) 
	{
		res = iconv(cd, &inptr, &insize, &outptr, &outsize);
		if (res == (size_t)(-1)) 
		{
			if (errno == EINVAL)
				break;
			else {
				int saved_errno = errno;
				iconv_close(cd);
				errno = saved_errno;
				return -1;
			}
		}
	}
	res = iconv(cd, NULL, NULL, &outptr, &outsize);
	if (res == (size_t) (-1)) 
	{
		saved_errno = errno;
		iconv_close(cd);
		errno = saved_errno;
		return -1;
	}
/* ??? */
/*    if (outsize != 0) abort(); */
	iconv_close(cd);
	*(result+length) = 0;
	if (is_utf16)
		*(result+length+1) = 0;
	return 0;
}
#endif

int iconv_string (const char* tocode, const char* fromcode,
				  const char* start, const char* end,
				  char** resultp, size_t* lengthp)
{
	size_t insize, outsize, res;
	char *result, *outptr;
	const char *inptr;
	iconv_t cd;

	insize = end - start;
	if (!strcmp(tocode, fromcode)) /* ����������� */
	{
		result = *resultp = (char*) (*resultp == NULL ? xmlMalloc(insize+2) : xmlRealloc(*resultp, insize+2));
		memcpy(*resultp, start, insize);
		result[insize] = result[insize+1] = 0; /* �� utf-16 */
		if (lengthp)
			*lengthp = insize;//+ 1;
		return 0;
	}
	cd = iconv_open(tocode, fromcode);
	if (cd == (iconv_t) (-1)) 
		return -1;
	result = (char*) xmlMalloc((end - start + 1)*sizeof(uint32_t)); /* �� 4 ����/������ */
	if (!result) 
	{
		iconv_close(cd);
		errno = ENOMEM;
		return -1;
	}
	inptr = start;
	outptr = result;
	outsize = insize*2;
	while (insize)
	{
		res = iconv(cd, (char**) &inptr, &insize, &outptr, &outsize);
		if (res == (size_t) - 1)
		{
			iconv_close(cd);
			if (resultp)
				*resultp = NULL;
			return -1;
		}
	}
	iconv_close(cd);
	/* ������� ���� */
	*outptr = *(outptr + 1) = 0;
	if (lengthp)
		*lengthp = outptr - result;
	result = (char*) xmlRealloc(result, outptr - result + 2);
	if (resultp)
		*resultp = result;
	return 0;
}

/* BASE64 */

#define uint8_t unsigned char
#define uint32_t unsigned int
/* ����� � http://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64 */
int base64encode(const void* data_buf, size_t dataLength, char* result, size_t resultSize)
{
	const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	const uint8_t *data = (const uint8_t*) data_buf;
	size_t result_index = 0;
	size_t x;
	uint32_t n = 0;
	int padCount = (int) (dataLength % 3);
	uint8_t n0, n1, n2, n3;
 
	/* increment over the length of the string, three characters at a time */
	for (x = 0; x < dataLength; x += 3) 
	{
		/* these three 8-bit (ASCII) characters become one 24-bit number */
		n = data[x] << 16; 
		if ((x+1) < dataLength)
			n += data[x+1] << 8; 
		if ((x+2) < dataLength)
			n += data[x+2]; 
		/* this 24-bit number gets separated into four 6-bit numbers */
		n0 = (uint8_t)(n >> 18) & 63;
		n1 = (uint8_t)(n >> 12) & 63;
		n2 = (uint8_t)(n >> 6) & 63;
		n3 = (uint8_t) n & 63; 
		/*
		 * if we have one byte available, then its encoding is spread
		 * out over two characters
		 */
		if (result_index >= resultSize) return 0;   /* indicate failure: buffer too small */
		result[result_index++] = base64chars[n0];
		if (result_index >= resultSize) return 0;   /* indicate failure: buffer too small */
		result[result_index++] = base64chars[n1]; 
		/*
		 * if we have only two bytes available, then their encoding is
		 * spread out over three chars
		 */
		if ((x+1) < dataLength)
		{
			if (result_index >= resultSize) return 0;   /* indicate failure: buffer too small */
			result[result_index++] = base64chars[n2];
		} 
		/*
		 * if we have all three bytes available, then their encoding is spread
		 * out over four characters
		 */
		if ((x+2) < dataLength)
		{
			if (result_index >= resultSize) return 0;   /* indicate failure: buffer too small */
			result[result_index++] = base64chars[n3];
		}
	}  
 	/*
	 * create and add padding that is required if we did not have a multiple of 3
	 * number of characters available
	 */
	if (padCount > 0) 
	{ 
		for (; padCount < 3; padCount++) 
		{ 
			if(result_index >= resultSize) return 0;   /* indicate failure: buffer too small */
			result[result_index++] = '=';
		} 
	}
	if (result_index >= resultSize) return 0;   /* indicate failure: buffer too small */
	result[result_index] = 0;
	return 1;   /* indicate success */
}

/* ������ ��, ������� � C++ */
#ifdef _USE_OLD_BASE64_DECODER
size_t base64decode(const void *input, size_t size, uint8_t* output, size_t output_size)
{
	const static uint8_t pad_character = '=';
	uint8_t *cursor = (uint8_t*) input;
	uint8_t *input_end = cursor + size;
	uint8_t *result_cursor = output;
	uint32_t temp = 0; /* Holds decoded quanta */
	size_t padding = 0;
	size_t quantum_pos;

	if (size % 4) 
		return (size_t) -1;
	if (!size) 
	{
		if (output_size)
		{
			*output = 0;
			return 0;
		}
		return (size_t) -1;
	}
	if (cursor[size-1] == pad_character)
		padding++;
	if (cursor[size-2] == pad_character)
		padding++;
	if (output_size < (((size >> 2)*3) - padding + 1))
		return (size_t) -1; /* insufficient memory */

	while (cursor < input_end)
	{
		for (quantum_pos = 0; quantum_pos < 4; quantum_pos++)
		{
			temp <<= 6;
			if (*cursor >= 0x41 && *cursor <= 0x5A)
				temp |= *cursor - 0x41;                        
			else if (*cursor >= 0x61 && *cursor <= 0x7A)
				temp |= *cursor - 0x47;
			else if (*cursor >= 0x30 && *cursor <= 0x39)
				temp |= *cursor + 0x04;
			else if (*cursor == 0x2B)
				temp |= 0x3E; /* change to 0x2D for URL alphabet */
			else if (*cursor == 0x2F)
				temp |= 0x3F; /* change to 0x5F for URL alphabet */
			else if (*cursor == pad_character)
			{
				switch (input_end - cursor)
				{
				case 1: /* one pad character */
					*result_cursor++ = ((temp >> 16) & 0x000000FF);
					*result_cursor++ = ((temp >> 8 ) & 0x000000FF);
					return result_cursor - output - 1;
				case 2: /* two pad characters */
					*result_cursor++ = ((temp >> 10) & 0x000000FF);
					return result_cursor - output - 1;
				default:
					return (size_t) -1;
				}
			}  else
				return (size_t) -1; /* non-valid data */
			cursor++;
		}
		*result_cursor++ = ((temp >> 16) & 0x000000FF);
		*result_cursor++ = ((temp >> 8 ) & 0x000000FF);
		*result_cursor++ = ((temp      ) & 0x000000FF);
	}
	return result_cursor - output - 1;
}
#else

#define WHITESPACE 64
#define EQUALS     65
#define INVALID    66

static const unsigned char base64_table[] = 
{
	66, 66, 66, 66, 66, 66, 66, 66, /* 0x00 - 0x07 */
	66, 64, 66, 66, 66, 66, 66, 66, /* 0x08 - 0x0F */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0x10 - 0x17 */
	66,	66, 66, 66, 66, 66, 66, 66, /* 0x18 - 0x1F */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0x20 - 0x27 */
	66, 66, 66, 62, 66, 66, 66, 63, /* 0x28 - 0x2F ...+.../ */
	52, 53,	54, 55, 56, 57, 58, 59, /* 0x30 - 0x37 01234567 */
	60, 61, 66, 66, 66, 65, 66, 66, /* 0x38 - 0x3F 89...=.. */
	66,  0,  1,  2,  3,  4,  5,  6, /* 0x40 - 0x47 .ABCDEFG */
	 7,  8,  9, 10, 11, 12, 13, 14, /* 0x48 - 0x4F HIJKLMNO */
	15, 16, 17, 18, 19, 20, 21, 22, /* 0x50 - 0x57 PQRSTUVW */
	23, 24, 25, 66, 66, 66, 66, 66, /* 0x58 - 0x5F XYZ..... */
	66, 26, 27, 28,	29, 30, 31, 32, /* 0x60 - 0x67 .abcdefg */
	33, 34, 35, 36, 37, 38, 39, 40, /* 0x68 - 0x6F hijklmno */
	41, 42, 43, 44, 45, 46, 47, 48, /* 0x70 - 0x77 pqrstuvw */
	49, 50, 51, 66, 66,	66, 66, 66, /* 0x78 - 0x7F xyz..... */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0x80 - 0x87 */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0x88 - 0x8F */
	66, 66, 66, 66, 66, 66,	66, 66, /* 0x90 - 0x97 */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0x98 - 0x9F */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xA0 - 0xA7 */
	66, 66, 66, 66, 66, 66, 66,	66, /* 0xA8 - 0xAF */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xB0 - 0xB7 */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xB8 - 0xBF */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xC0 - 0xC7 */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xC8 - 0xCF */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xD0 - 0xD7 */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xD8 - 0xDF */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xE0 - 0xE7 */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xE8 - 0xEF */
	66, 66, 66, 66, 66, 66, 66, 66, /* 0xF0 - 0xF7 */
	66, 66,	66, 66, 66, 66, 66, 66  /* 0xF8 - 0xFF */
};

size_t base64decode (const char *data_buf, size_t dataLength, char *result, size_t resultSize) 
{ 
	const char *end = data_buf + dataLength;
	size_t buf = 1, len = 0;

	while (data_buf < end) 
	{
		unsigned char c = base64_table[*data_buf++];

		switch (c) 
		{
		case WHITESPACE: continue;   /* skip whitespace */
		case INVALID:    return (size_t) -1;   /* invalid input, return error */
		case EQUALS:                 /* pad character, end of data */
			data_buf = end;
			continue;
		default:
			buf = buf << 6 | c;
			/* If the buffer is full, split it into bytes */
			if (buf & 0x1000000) /* 1 - ��� "1" �� �������������! */
			{
				if ((len += 3) > resultSize) 
					return (size_t) -1; /* buffer overflow */
				*result++ = (buf >> 16) & 0xFF;
				*result++ = (buf >> 8)  & 0xFF;
				*result++ = buf & 0xFF;
				buf = 1;
			}   
		}
	}

	if (buf & 0x40000) 
	{
		if ((len += 2) > resultSize) 
			return (size_t) -1; /* buffer overflow */
		*result++ = (buf >> 10) & 0xFF;
		*result++ = (buf >> 2)  & 0xFF;
	} else if (buf & 0x1000) {
		if (++len > resultSize) 
			return (size_t) -1; /* buffer overflow */
		*result++ = (buf >> 4) & 0xFF;
	}
	return len;
}
#endif

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

xmlChar* encodeUriIdn(xmlChar *uri)
{
#ifdef _USE_LIBIDN
	xmlChar *ret = NULL, *slash_pos;
	char *enc_domain;
	slash_pos = BAD_CAST xmlStrchr(uri, '/');
	if (!slash_pos || (slash_pos == uri)) /* uri doesn't contain '/' or starts with '/' */
		return NULL;
	if (*(slash_pos - 1) != ':')
		return NULL; /* not ':/' */
	if (*(++slash_pos) != '/')
		return NULL; /* not '//' */
	idna_to_ascii_8z((char*) slash_pos+1, &enc_domain, 0);
	if (!enc_domain)
		return NULL;
	/* ���������� URI ������� 4 �� */
	ret = xmlStrndup(uri, (int)(slash_pos - uri + 1));
	ret = xmlStrcat(ret, BAD_CAST enc_domain);
#ifdef _WIN32
	idn_free(enc_domain);
#else
	free(enc_domain);
#endif
	return ret;
#else
	return xmlStrdup(uri);
#endif
}

size_t getOffsetToNextUTF8Char(xmlChar *cur)
{
	if (!cur || !*cur)
		return 0;
	/* 1 ������ != 1 ����! */
	if (!(*cur & 0x80))
		return 1;
	else if ((*cur & 0xE0) == 0xC0)
		return 2;
	else if ((*cur & 0xF0) == 0xE0)
		return 3;
	else if ((*cur & 0xF8) == 0xF0)
		return 4;
	/* �� � �������� �������, �������� �� ������� ��������� */
	return getOffsetToNextUTF8Char(cur + 1) + 1;
}

void composeAndSplitPath(xmlChar *basePath, xmlChar *relativePath, xmlChar **normalizedPath, xmlChar **normalizedFilename)
{
	size_t base_path_len = xmlStrlen(basePath);

	*normalizedFilename = BAD_CAST strrchr((const char*) relativePath, '/');
	if (*normalizedFilename)
	{
		(*normalizedFilename)++;
		*normalizedPath = (xmlChar*) xmlMalloc(xmlStrlen(basePath) + (*normalizedFilename - relativePath) + 1);
		if ((basePath[base_path_len - 1] != XPR_PATH_DELIM) && (basePath[base_path_len - 1] != XPR_PATH_INVERSE_DELIM))
			strcpy((char*) *normalizedPath, (char*) basePath);
		else {
			strncpy((char*) *normalizedPath, (char*) basePath, base_path_len - 1);
			(*normalizedPath)[base_path_len - 1] = 0;
		}
		strncat((char*) *normalizedPath, (char*) relativePath, *normalizedFilename - relativePath);
	} else {
		*normalizedFilename = relativePath;
		*normalizedPath = xmlStrdup(basePath);
	}
	if (*normalizedPath)
		xprConvertSlashes(*normalizedPath);
	/*printf("*** %s %s => %s %s\n", basePath, relativePath, *normalizedPath, *normalizedFilename);*/
}

xmlNsPtr getResultingNs(xmlNodePtr parent, xmlNodePtr invoker, xmlChar *name)
{
	xmlNsPtr ret = xmlSearchNs(parent->doc, parent, name);
	if (ret)
		return ret;
	ret = xmlSearchNs(invoker->doc, invoker, name);
	if (ret)
		return xmlNewNs(parent, ret->href, ret->prefix);
	return NULL;
}

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
		else 
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

xmlChar* appendThreadIdToString(xmlChar *str, XPR_THREAD_ID id)
{
	size_t len;
	char buf[17];
	
	len = xmlStrlen(str);
	str = (xmlChar*) xmlRealloc(str, len + sizeof(buf));
	if (!str)
		return NULL;
	/* TODO x64 */
	snprintf(buf, 17, XPR_THREAD_ID_FORMAT, id);
	strcpy((char*) (str+len), buf);
	return str;
}
