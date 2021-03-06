/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2021     */
/******************************************/
#ifndef __xpltree_H
#define __xpltree_H

#include "Configuration.h"
#include <stdbool.h>
#include <libxml/tree.h>
#include <libxml/xmlstring.h>
#include <libxml/xpath.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _xplQName
{
	xmlNsPtr ns;
	xmlChar *ncname;
} xplQName, *xplQNamePtr;

#define XPL_XML_PARSE_OPTIONS (XML_PARSE_NOENT | XML_PARSE_NODICT)

#define XPL_NODE_DELETION_REQUEST_FLAG ((xmlElementType) 0x0080UL)
#define XPL_NODE_DELETION_DEFERRED_FLAG ((xmlElementType) 0x0100UL)
#define XPL_NODE_DELETION_MASK (XPL_NODE_DELETION_DEFERRED_FLAG | XPL_NODE_DELETION_REQUEST_FLAG)

typedef enum _xplParseQNameResult
{
	XPL_PARSE_QNAME_OK,
	XPL_PARSE_QNAME_INVALID_QNAME,
	XPL_PARSE_QNAME_UNKNOWN_NS
} xplParseQNameResult;

XPLPUBFUN xplParseQNameResult XPLCALL
	xplParseQName(const xmlChar *str, const xmlNodePtr element, xplQNamePtr qname);
XPLPUBFUN xmlChar* XPLCALL
	xplQNameToStr(const xplQName qname);
XPLPUBFUN void XPLCALL
	xplClearQName(xplQNamePtr qname);

#define APPEND_NODE_TO_LIST(head, tail, cur) \
	do { \
		if ((tail)) \
		{ \
			(tail)->next = (cur); \
			(cur)->prev = (tail); \
			(tail) = (cur); \
		} else \
			(head) = (tail) = (cur); \
	} while(0)

/* locates node list tail */
XPLPUBFUN xmlNodePtr XPLCALL
	xplFindTail(const xmlNodePtr head);
/* ditto */
XPLPUBFUN xmlNodePtr XPLCALL
	xplFirstElementNode(const xmlNodePtr list);
/* checks if maybeAncestor is ancestor of maybeChild */
XPLPUBFUN bool XPLCALL
	xplIsAncestor(const xmlNodePtr maybeChild, const xmlNodePtr maybeAncestor);

/* returns a flattened copy of attribute value */
XPLPUBFUN xmlChar* XPLCALL
	xplGetPropValue(const xmlAttrPtr prop);
/* unlinks a property but doesn't free it */
XPLPUBFUN void XPLCALL
	xplUnlinkProp(xmlAttrPtr cur);

/* detaches el's children */
XPLPUBFUN xmlNodePtr XPLCALL
	xplDetachChildren(xmlNodePtr el);
/* sets el's children to list, returns list's last node */
XPLPUBFUN xmlNodePtr XPLCALL
	xplSetChildren(xmlNodePtr el, xmlNodePtr list);
/* appends list to el's children, returns list's last node */
XPLPUBFUN xmlNodePtr XPLCALL
	xplAppendChildren(xmlNodePtr el, xmlNodePtr list);
/* prepends list to el's children, returns list's last node */
XPLPUBFUN xmlNodePtr XPLCALL
	xplPrependChildren(xmlNodePtr el, xmlNodePtr list);
/* adds nsDef to the end of cur's nsDef list */
XPLPUBFUN void XPLCALL
	xplAppendNsDef(xmlNodePtr cur, xmlNsPtr ns);

/* inserts list after el, returns list's last node */
XPLPUBFUN xmlNodePtr XPLCALL
	xplAppendList(xmlNodePtr el, xmlNodePtr list);
/* inserts list before el, returns list's last node */
XPLPUBFUN xmlNodePtr XPLCALL
	xplPrependList(xmlNodePtr el, xmlNodePtr list);
/* replaces el with list, returns el */
XPLPUBFUN xmlNodePtr XPLCALL
	xplReplaceWithList(xmlNodePtr el, xmlNodePtr list);

/* checks for text/CDATA/entity refs only */
XPLPUBFUN bool XPLCALL
	xplCheckNodeListForText(const xmlNodePtr start);
XPLPUBFUN bool XPLCALL
	xplCheckNodeSetForText(const xmlNodeSetPtr s);

/* marks ancestor axis part from bottom to top (both inclusive) for deferred deletion */
XPLPUBFUN void XPLCALL 
	xplMarkAncestorAxisForDeletion(xmlNodePtr bottom, xmlNodePtr top);
/* mark/unmark cur, its props and all of its descendants for deletion.
   if doMark, set bitwiseAttribute, otherwise clear it. */
XPLPUBFUN void XPLCALL
	xplMarkDOSAxisForDeletion(xmlNodePtr cur, int bitwiseAttribute, bool doMark);
/* delete everything but ancestor axis starting at cur and stopping at boundary (exclusive).
   if markAncestorAxis is set, also mark the nodes on the axis for deletion. */
XPLPUBFUN void XPLCALL
	xplDeleteNeighbours(xmlNodePtr cur, xmlNodePtr boundary, bool markAncestorAxis);

/* Initialize the copying mechanism internals */
XPLPUBFUN bool XPLCALL
	xplInitNamePointers(void);
/* Copy nodes or node lists wrt their hierarchy.
   parent is only used for namespace search and IS NOT assigned to new nodes */
XPLPUBFUN xmlNodePtr XPLCALL
	xplCloneNode(const xmlNodePtr node, const xmlNodePtr parent, const xmlDocPtr doc);
XPLPUBFUN xmlNodePtr XPLCALL
	xplCloneNodeList(const xmlNodePtr node, const xmlNodePtr parent, const xmlDocPtr doc);
/* clone a node changing attributes into text nodes */
XPLPUBFUN xmlNodePtr XPLCALL
	xplCloneAsNodeChild(const xmlNodePtr cur, const xmlNodePtr parent);

/* Copy all higher namespace definitions to top. Returns false on OOM */
XPLPUBFUN bool XPLCALL
	xplMakeNsSelfContainedTree(xmlNodePtr top);
/* Translate top's nsDefs one level up removing redundant ones. Returns false on OOM */
XPLPUBFUN bool XPLCALL
	xplLiftNsDefs(xmlNodePtr parent, xmlNodePtr carrier, xmlNodePtr children);
/* Ensure no links to from->oldNs remain inside its child tree */
XPLPUBFUN bool XPLCALL
	xplMergeDocOldNamespaces(xmlDocPtr from, xmlDocPtr to);

  
XPLPUBFUN void XPLCALL
	xplRegisterXPathExtensions(xmlXPathContextPtr ctxt);

/* Comparison.
   Identity: exactly the same nodes
   Equality: node names/contents/props must be equal and written in the same order (except props) */
XPLPUBFUN bool XPLCALL
	xplCheckNodeEquality(const xmlNodePtr a, const xmlNodePtr b);
XPLPUBFUN bool XPLCALL
	xplCheckNodeListEquality(const xmlNodePtr a, const xmlNodePtr b);
XPLPUBFUN bool XPLCALL
	xplCheckPropListEquality(const xmlAttrPtr a, const xmlAttrPtr b);
/* the following functions are using Cartesian products */
XPLPUBFUN bool XPLCALL
	xplCheckNodeSetEquality(const xmlNodeSetPtr a, const xmlNodeSetPtr b);
XPLPUBFUN bool XPLCALL
	xplCheckNodeSetIdentity(const xmlNodeSetPtr a, const xmlNodeSetPtr b);
XPLPUBFUN bool XPLCALL
	xplCompareXPathSelections(const xmlXPathObjectPtr a, const xmlXPathObjectPtr b, bool checkEquality);

XPLPUBFUN xmlXPathObjectPtr XPLCALL
	xplSelectNodesWithCtxt(xmlXPathContextPtr ctxt, const xmlNodePtr src, const xmlChar *expr);

XPLPUBFUN xmlAttrPtr XPLCALL
	xplCreateAttribute(xmlNodePtr dst, const xplQName qname, const xmlChar *value, bool allowReplace);

XPLPUBFUN xmlNodeSetPtr XPLCALL
	xplGetNodeAncestorOrSelfAxis(xmlNodePtr cur);
XPLPUBFUN bool XPLCALL
	xplVerifyAncestorOrSelfAxis(const xmlNodePtr root, const xmlNodeSetPtr axis);

#ifdef __cplusplus
}
#endif
#endif
