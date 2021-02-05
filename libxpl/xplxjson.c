#include <stdbool.h>
#include <libxpl/xplbuffer.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>
#include <libxpl/xplxjson.h>

typedef enum _xjsonElementType {
	XJE_ARRAY,
	XJE_OBJECT,
	XJE_STRING,
	XJE_NUMBER,
	XJE_BOOLEAN,
	XJE_NULL,
	XJE_NONE
} xjsonElementType;

#define XJSON_TYPE_IS_CONTAINER(t) ((t) == XJE_OBJECT || (t) == XJE_ARRAY)

typedef struct _xjsonSerializeCtxt {
	xmlNodePtr parent;
	bool strict_tag_names;
	bool value_type_check;
	rbBufPtr buf;
	/* will be changed by serializer */
	xmlNsPtr xjson_ns;
	xjsonElementType container_type;
	bool is_first_item;
} xjsonSerializeCtxt, *xjsonSerializeCtxtPtr;

static bool _checkJsonNs(xmlNsPtr ns, xmlNsPtr *cached_ns)
{
	if (!ns)
		return false;
	if (ns == *cached_ns)
		return true;
	if (!ns->href)
		return false;
	if (xmlStrcmp(ns->href, XJSON_SCHEMA_URI))
		return false;
	if (!*cached_ns)
		*cached_ns = ns;
	return true;
}

static xjsonElementType _getElementType(xmlNodePtr cur)
{
	if (!xmlStrcmp(cur->name, BAD_CAST "array"))
		return XJE_ARRAY;
	else if (!xmlStrcmp(cur->name, BAD_CAST "object"))
		return XJE_OBJECT;
	else if (!xmlStrcmp(cur->name, BAD_CAST "string"))
		return XJE_STRING;
	else if (!xmlStrcmp(cur->name, BAD_CAST "number"))
		return XJE_NUMBER;
	else if (!xmlStrcmp(cur->name, BAD_CAST "boolean"))
		return XJE_BOOLEAN;
	else if (!xmlStrcmp(cur->name, BAD_CAST "null"))
		return XJE_NULL;
	return XJE_NONE;
}

static xmlNodePtr _createMemoryError(xjsonSerializeCtxtPtr ctxt)
{
	return xplCreateErrorNode(ctxt->parent, BAD_CAST "insufficient memory");
}

#define ADD_DATA(d, size) \
	do { \
		if (rbAddDataToBuf(ctxt->buf, d, size) != RB_RESULT_OK) \
		{ \
			ret = _createMemoryError(ctxt); \
			goto done; \
		} \
	} while (0)

static xmlNodePtr _xjsonSerializeNodeList(xmlNodePtr first, xjsonSerializeCtxtPtr ctxt);

static xmlNodePtr _xjsonSerializeList(xmlNodePtr cur, xjsonSerializeCtxtPtr ctxt, xjsonElementType containerType)
{
	xjsonElementType prev_container_type;
	bool prev_is_first_item;
	xmlNodePtr ret = NULL;

	if (rbAddDataToBuf(ctxt->buf, (containerType == XJE_ARRAY)? "[": "{", 1) != RB_RESULT_OK)
		return _createMemoryError(ctxt);
	prev_container_type = ctxt->container_type;
	prev_is_first_item = ctxt->is_first_item;
	ctxt->container_type = containerType;
	ctxt->is_first_item = true;
	if ((ret = _xjsonSerializeNodeList(cur->children, ctxt)))
		goto done;
	ADD_DATA(containerType == XJE_ARRAY? "]": "}", 1);
done:
	ctxt->container_type = prev_container_type;
	ctxt->is_first_item = prev_is_first_item;
	return ret;
}

static xmlNodePtr _xjsonSerializeAtom(xmlNodePtr cur, xjsonSerializeCtxtPtr ctxt, xjsonElementType type)
{
	xmlNodePtr ret = NULL;
	xmlChar *content = NULL;

	if (!xplCheckNodeListForText(cur->children))
		return xplCreateErrorNode(ctxt->parent, BAD_CAST "element '%s' has non-text nodes inside", cur->name);
	if (type == XJE_STRING)
		ADD_DATA("\"", 1);
	content = xmlNodeListGetString(cur->doc, cur->children, 1);
	if (ctxt->value_type_check)
		switch(type)
		{
		case XJE_STRING:
			break;
		case XJE_NUMBER:
			if (!xstrIsNumber(content))
			{
				ret = xplCreateErrorNode(ctxt->parent, BAD_CAST "element '%s' content '%s' is non-numeric", cur->name, content);
				goto done;
			}
			break;
		case XJE_BOOLEAN:
			if (xmlStrcmp(content, BAD_CAST "false") && xmlStrcmp(content, BAD_CAST "true"))
			{
				ret = xplCreateErrorNode(ctxt->parent, BAD_CAST "element '%s' content '%s' is non-boolean", cur->name, content);
				goto done;
			}
			break;
		case XJE_NULL:
			if (content && *content)
			{
				ret = xplCreateErrorNode(ctxt->parent, BAD_CAST "element '%s' content '%s' is non-empty", cur->name, content);
				goto done;
			}
			break;
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
		}
	ADD_DATA(type == XJE_NULL? BAD_CAST "null": content, xmlStrlen(type == XJE_NULL? BAD_CAST "null": content));
	if (type == XJE_STRING)
		ADD_DATA("\"", 1);
done:
	if (content)
		XPL_FREE(content);
	return ret;
}

static xmlNodePtr _xjsonSerializeNode(xmlNodePtr cur, xjsonSerializeCtxtPtr ctxt)
{
	xmlChar *name = NULL;
	xmlNodePtr ret = NULL;
	xjsonElementType el_type;

	if (!cur->ns || !_checkJsonNs(cur->ns, &ctxt->xjson_ns))
	{
		if (ctxt->strict_tag_names)
			return xplCreateErrorNode(ctxt->parent, BAD_CAST "element '%s:%s' is not in XJSON namespace", cur->ns? cur->ns->prefix: NULL, cur->name);
		return NULL;
	}
	el_type = _getElementType(cur);
	if (el_type == XJE_NONE)
	{
		if (ctxt->strict_tag_names)
			return xplCreateErrorNode(ctxt->parent, BAD_CAST "unknown tag name '%s'", cur->name);
		return NULL;
	}
	if (ctxt->container_type != XJE_NONE)
	{
		if (!ctxt->is_first_item)
			ADD_DATA(",", 1);
		else
			ctxt->is_first_item = false;
	}
	name = xmlGetNoNsProp(cur, BAD_CAST "name");
	if (name)
	{
		if (ctxt->container_type != XJE_OBJECT)
		{
			ret = xplCreateErrorNode(ctxt->parent, BAD_CAST "cannot use named items outside of object");
			goto done;
		}
		ADD_DATA("\"", 1);
		ADD_DATA(name, xmlStrlen(name));
		ADD_DATA("\":", 2);
	} else if (ctxt->container_type == XJE_OBJECT) {
		ret = xplCreateErrorNode(ctxt->parent, BAD_CAST "items inside an object must be named");
		goto done;
	}

	if (XJSON_TYPE_IS_CONTAINER(el_type))
		ret = _xjsonSerializeList(cur, ctxt, el_type);
	else
		ret = _xjsonSerializeAtom(cur, ctxt, el_type);
done:
	if (name)
		XPL_FREE(name);
	return ret;
}

static xmlNodePtr _xjsonSerializeNodeList(xmlNodePtr first, xjsonSerializeCtxtPtr ctxt)
{
	xmlNodePtr error;

	while (first)
	{
		if ((error = _xjsonSerializeNode(first, ctxt)))
			return error;
		first = first->next;
	}
	return NULL;
}

xmlNodePtr xplXJsonSerializeNodeList(xmlNodePtr list, bool strictTagNames, bool valueTypeCheck)
{
	xjsonSerializeCtxt ctxt;
	xmlNodePtr ret;

	ctxt.buf = rbCreateBufParams(1024, RB_GROW_DOUBLE, 2);
	if (!ctxt.buf)
		return NULL;
	ctxt.strict_tag_names = strictTagNames;
	ctxt.value_type_check = valueTypeCheck;
	ctxt.xjson_ns = NULL;
	ctxt.parent = list->parent;
	ctxt.container_type = XJE_NONE;
	ctxt.is_first_item = true;

	if ((ret = _xjsonSerializeNodeList(list, &ctxt)))
		goto done;

	if (rbAddDataToBuf(ctxt.buf, "", 1) != RB_RESULT_OK) // zero-terminate
	{
		ret = _createMemoryError(&ctxt);
		goto done;
	}

	ret = xmlNewDocText(list->doc, NULL);
	ret->content = rbDetachBufContent(ctxt.buf);
done:
	if (ctxt.buf)
		rbFreeBuf(ctxt.buf);
	return ret;
}
