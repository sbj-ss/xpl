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

#define EXTRACT_NS_AND_TAGNAME(src, ns, tagName, parent) do { \
	tagName = BAD_CAST xmlStrchr((src), ':'); \
	if (tagName) \
	{ \
		*tagName = 0; \
		ns = xmlSearchNs((parent)->doc, (parent), (src)); \
		*tagName++ = ':'; \
	} else { \
		tagName = BAD_CAST (src); \
		ns = NULL; \
	} \
} while(0);

#define XML_NODE_DELETION_REQUEST_FLAG ((xmlElementType) 0x0080UL)
#define XML_NODE_DELETION_DEFERRED_FLAG ((xmlElementType) 0x0100UL)
#define XML_NODE_DELETION_MASK (XML_NODE_DELETION_DEFERRED_FLAG | XML_NODE_DELETION_REQUEST_FLAG)

/* locates node list tail */
XPLPUBFUN xmlNodePtr XPLCALL
	findTail(xmlNodePtr cur);
/* checks if maybeAncestor is ancestor of maybeChild */
XPLPUBFUN bool XPLCALL
	isAncestor(xmlNodePtr maybeChild, xmlNodePtr maybeAncestor);
/* if ns is declared on invoker but invalid on parent - a dupe is created */
XPLPUBFUN xmlNsPtr XPLCALL
	getResultingNs(xmlNodePtr parent, xmlNodePtr invoker, xmlChar *name);

/* returns a flattened copy of attribute value */
XPLPUBFUN xmlChar* XPLCALL
	getPropValue(xmlAttrPtr prop);
/* unlinks a property but doesn't free it */
XPLPUBFUN void XPLCALL
	unlinkProp(xmlAttrPtr cur);
/* attaches new property to dst. src is used for namespace search. name may be a QName */
XPLPUBFUN void XPLCALL
	assignAttribute(xmlNodePtr src, xmlNodePtr dst, xmlChar *name, xmlChar *value, bool allowReplace);

/* creates an element. invoker is used for namespace search. name may be a QName */
XPLPUBFUN xmlNodePtr XPLCALL
	createElement(xmlNodePtr parent, xmlNodePtr invoker, xmlChar *name);

/* detaches el's children */
XPLPUBFUN xmlNodePtr XPLCALL /* TODO rename */
	detachContent(xmlNodePtr el);
/* sets el's children to list, returns list's last node */
XPLPUBFUN xmlNodePtr XPLCALL
	setChildren(xmlNodePtr el, xmlNodePtr list);
/* appends list to el's children, returns list's last node */
XPLPUBFUN xmlNodePtr XPLCALL
	appendChildren(xmlNodePtr el, xmlNodePtr list);

/* inserts list after el, returns list's last node */
XPLPUBFUN xmlNodePtr XPLCALL
	appendList(xmlNodePtr el, xmlNodePtr list);
/* inserts list before el, returns list's last node */
XPLPUBFUN xmlNodePtr XPLCALL
	prependList(xmlNodePtr el, xmlNodePtr list);
/* replaces el with list, returns el */
XPLPUBFUN xmlNodePtr XPLCALL
	replaceWithList(xmlNodePtr el, xmlNodePtr list);

/* Добавить следующий элемент с трансформацией атрибута в текст (после XPath-выборки) *//* TODO rename */
XPLPUBFUN xmlNodePtr XPLCALL
	cloneAttrAsText(xmlNodePtr cur, xmlNodePtr parent);

/* checks for text/CDATA/entity refs only */
XPLPUBFUN bool XPLCALL
	checkNodeListForText(xmlNodePtr start);
XPLPUBFUN bool XPLCALL
	checkNodeSetForText(xmlNodeSetPtr s);

/* marks ancestor axis part from bottom to top (both inclusive) for deferred deletion */
XPLPUBFUN void XPLCALL 
	markAncestorAxisForDeletion(xmlNodePtr bottom, xmlNodePtr top);
/* mark/unmark cur, its props and all of its descendants for deletion.
   if doMark, set bitwiseAttribute, otherwise clear it. */
XPLPUBFUN void XPLCALL
	markDOSAxisForDeletion(xmlNodePtr cur, int bitwiseAttribute, bool doMark);
/* delete everything but ancestor axis starting at cur and stopping at boundary (exclusive).
   if markAncestorAxis is set, also mark the nodes on the axis for deletion. */
XPLPUBFUN void XPLCALL
	deleteNeighbours(xmlNodePtr cur, xmlNodePtr boundary, bool markAncestorAxis);

/* Copy nodes or node lists wrt their hierarchy.
   parent is only used for namespace search and IS NOT assigned to new nodes */
XPLPUBFUN bool XPLCALL
	initNamePointers(void);
XPLPUBFUN xmlNodePtr XPLCALL
	cloneNode(xmlNodePtr node, xmlNodePtr parent, xmlDocPtr doc);
XPLPUBFUN xmlNodePtr XPLCALL
	cloneNodeList(xmlNodePtr node, xmlNodePtr parent, xmlDocPtr doc);

/* Translate parent's ns_list to cur before deleting its parent. Assume cur is already detached. */
XPLPUBFUN void XPLCALL
	downshiftNodeNsDef(xmlNodePtr cur, xmlNsPtr ns_list);
XPLPUBFUN void XPLCALL
	downshiftNodeListNsDef(xmlNodePtr cur, xmlNsPtr ns_list);

/* Copy all higher namespace definitions to top */
XPLPUBFUN void XPLCALL
	makeNsIndepTree(xmlNodePtr top);
  
XPLPUBFUN void XPLCALL
	xplRegisterXPathExtensions(xmlXPathContextPtr ctxt);

/* Comparison.
   Identity: exactly the same nodes
   Equality: node names/contents/props must be equal and written in the same order (except props) */
XPLPUBFUN bool XPLCALL
	checkNodeEquality(xmlNodePtr a, xmlNodePtr b);
XPLPUBFUN bool XPLCALL
	checkNodeListEquality(xmlNodePtr a, xmlNodePtr b);
XPLPUBFUN bool XPLCALL
	checkPropListEquality(xmlAttrPtr a, xmlAttrPtr b);
/* the following functions are using Cartesian products */
XPLPUBFUN bool XPLCALL
	checkNodeSetEquality(xmlNodeSetPtr a, xmlNodeSetPtr b);
XPLPUBFUN bool XPLCALL
	checkNodeSetIdentity(xmlNodeSetPtr a, xmlNodeSetPtr b);
XPLPUBFUN bool XPLCALL
	compareXPathSelections(xmlXPathObjectPtr a, xmlXPathObjectPtr b, bool checkEquality);

#ifdef __cplusplus
}
#endif
#endif
