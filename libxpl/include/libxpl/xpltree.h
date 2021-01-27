/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
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
	xplParseQName(xmlChar *str, xmlNodePtr element, xplQNamePtr qname);
XPLPUBFUN xmlChar* XPLCALL
	xplQNameToStr(xplQName qname);
XPLPUBFUN void XPLCALL
	xplClearQName(xplQNamePtr qname);

/* locates node list tail */
XPLPUBFUN xmlNodePtr XPLCALL
	xplFindTail(xmlNodePtr cur);
/* checks if maybeAncestor is ancestor of maybeChild */
XPLPUBFUN bool XPLCALL
	xplIsAncestor(xmlNodePtr maybeChild, xmlNodePtr maybeAncestor);
/* if ns is declared on invoker but invalid on parent - a dupe is created */
XPLPUBFUN xmlNsPtr XPLCALL
	xplGetResultingNs(xmlNodePtr parent, xmlNodePtr invoker, xmlChar *name);

/* returns a flattened copy of attribute value */
XPLPUBFUN xmlChar* XPLCALL
	xplGetPropValue(xmlAttrPtr prop);
/* unlinks a property but doesn't free it */
XPLPUBFUN void XPLCALL
	xplUnlinkProp(xmlAttrPtr cur);

/* detaches el's children */
XPLPUBFUN xmlNodePtr XPLCALL /* TODO rename */
	xplDetachContent(xmlNodePtr el);
/* sets el's children to list, returns list's last node */
XPLPUBFUN xmlNodePtr XPLCALL
	xplSetChildren(xmlNodePtr el, xmlNodePtr list);
/* appends list to el's children, returns list's last node */
XPLPUBFUN xmlNodePtr XPLCALL
	xplAppendChildren(xmlNodePtr el, xmlNodePtr list);

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
	xplCheckNodeListForText(xmlNodePtr start);
XPLPUBFUN bool XPLCALL
	xplCheckNodeSetForText(xmlNodeSetPtr s);

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
	xplCloneNode(xmlNodePtr node, xmlNodePtr parent, xmlDocPtr doc);
XPLPUBFUN xmlNodePtr XPLCALL
	xplCloneNodeList(xmlNodePtr node, xmlNodePtr parent, xmlDocPtr doc);
/* clone a node changing attributes into text nodes */
XPLPUBFUN xmlNodePtr XPLCALL
	xplCloneAsNodeChild(xmlNodePtr cur, xmlNodePtr parent);

/* Copy all higher namespace definitions to top. Returns false on OOM */
XPLPUBFUN bool XPLCALL
	xplMakeNsSelfContainedTree(xmlNodePtr top);
/* Translate top's nsDefs one level up removing redundant ones. Returns false on OOM */
XPLPUBFUN bool XPLCALL
	xplLiftNsDefs(xmlNodePtr top);
  
XPLPUBFUN void XPLCALL
	xplRegisterXPathExtensions(xmlXPathContextPtr ctxt);

/* Comparison.
   Identity: exactly the same nodes
   Equality: node names/contents/props must be equal and written in the same order (except props) */
XPLPUBFUN bool XPLCALL
	xplCheckNodeEquality(xmlNodePtr a, xmlNodePtr b);
XPLPUBFUN bool XPLCALL
	xplCheckNodeListEquality(xmlNodePtr a, xmlNodePtr b);
XPLPUBFUN bool XPLCALL
	xplCheckPropListEquality(xmlAttrPtr a, xmlAttrPtr b);
/* the following functions are using Cartesian products */
XPLPUBFUN bool XPLCALL
	xplCheckNodeSetEquality(xmlNodeSetPtr a, xmlNodeSetPtr b);
XPLPUBFUN bool XPLCALL
	xplCheckNodeSetIdentity(xmlNodeSetPtr a, xmlNodeSetPtr b);
XPLPUBFUN bool XPLCALL
	xplCompareXPathSelections(xmlXPathObjectPtr a, xmlXPathObjectPtr b, bool checkEquality);

XPLPUBFUN xmlXPathObjectPtr XPLCALL
	xplSelectNodesWithCtxt(xmlXPathContextPtr ctxt, xmlNodePtr src, xmlChar *expr);

XPLPUBFUN xmlAttrPtr XPLCALL
	xplCreateAttribute(xmlNodePtr dst, xplQName qname, xmlChar *value, bool allowReplace);

#ifdef __cplusplus
}
#endif
#endif
