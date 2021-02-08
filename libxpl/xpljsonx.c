#include <stdbool.h>
#include <libxpl/xplbuffer.h>
#include <libxpl/xpljsonx.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>

typedef enum _jsonxElementType {
	JXE_ARRAY,
	JXE_OBJECT,
	JXE_STRING,
	JXE_NUMBER,
	JXE_BOOLEAN,
	JXE_NULL,
	JXE_NONE
} jsonxElementType;

#define JSONX_TYPE_IS_CONTAINER(t) ((t) == JXE_OBJECT || (t) == JXE_ARRAY)

typedef struct _jsonxSerializeCtxt {
	xmlNodePtr parent;
	bool strict_tag_names;
	bool value_type_check;
	rbBufPtr buf;
	/* will be changed by serializer */
	xmlNsPtr jsonx_ns;
	jsonxElementType container_type;
	bool is_first_item;
} jsonxSerializeCtxt, *jsonxSerializeCtxtPtr;

static bool _checkJsonXNs(xmlNsPtr ns, xmlNsPtr *cached_ns)
{
	if (!ns)
		return false;
	if (ns == *cached_ns)
		return true;
	if (!ns->href)
		return false;
	if (xmlStrcmp(ns->href, JSONX_SCHEMA_URI))
		return false;
	if (!*cached_ns)
		*cached_ns = ns;
	return true;
}

static jsonxElementType _getElementType(xmlNodePtr cur)
{
	if (!xmlStrcmp(cur->name, BAD_CAST "array"))
		return JXE_ARRAY;
	else if (!xmlStrcmp(cur->name, BAD_CAST "object"))
		return JXE_OBJECT;
	else if (!xmlStrcmp(cur->name, BAD_CAST "string"))
		return JXE_STRING;
	else if (!xmlStrcmp(cur->name, BAD_CAST "number"))
		return JXE_NUMBER;
	else if (!xmlStrcmp(cur->name, BAD_CAST "boolean"))
		return JXE_BOOLEAN;
	else if (!xmlStrcmp(cur->name, BAD_CAST "null"))
		return JXE_NULL;
	return JXE_NONE;
}

static xmlNodePtr _createMemoryError(jsonxSerializeCtxtPtr ctxt)
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

static xmlChar esc_map[256] = {
	['"'] = '"',
	['/'] = '/',
	['\\'] = '\\',
	['\r'] = 'r',
	['\n'] = 'n',
	['\t'] = 't',
	['\b'] = 'b',
	['\f'] = 'f'
};

static xmlChar *_jsonxEscapeString(const xmlChar *s)
{
	int extra = 0;
	const xmlChar *p = s;
	xmlChar *ret, *cur;

	while (*p)
		extra += !!esc_map[*p++];
	if (!(ret = cur = BAD_CAST XPL_MALLOC(p - s + extra + 1)))
		return NULL;
	p = s;
	while (*p)
	{
		if (esc_map[*p])
		{
			*cur++ = '\\';
			*cur++ = esc_map[*p++];
		} else
			*cur++ = *p++;
	}
	*cur = 0;
	return ret;
}

static xmlNodePtr _jsonxSerializeNodeList(xmlNodePtr first, jsonxSerializeCtxtPtr ctxt);

static xmlNodePtr _jsonxSerializeList(xmlNodePtr cur, jsonxSerializeCtxtPtr ctxt, jsonxElementType containerType)
{
	jsonxElementType prev_container_type;
	bool prev_is_first_item;
	xmlNodePtr ret = NULL;

	if (rbAddDataToBuf(ctxt->buf, (containerType == JXE_ARRAY)? "[": "{", 1) != RB_RESULT_OK)
		return _createMemoryError(ctxt);
	prev_container_type = ctxt->container_type;
	prev_is_first_item = ctxt->is_first_item;
	ctxt->container_type = containerType;
	ctxt->is_first_item = true;
	if ((ret = _jsonxSerializeNodeList(cur->children, ctxt)))
		goto done;
	ADD_DATA(containerType == JXE_ARRAY? "]": "}", 1);
done:
	ctxt->container_type = prev_container_type;
	ctxt->is_first_item = prev_is_first_item;
	return ret;
}

static xmlNodePtr _jsonxSerializeAtom(xmlNodePtr cur, jsonxSerializeCtxtPtr ctxt, jsonxElementType type)
{
	xmlNodePtr ret = NULL;
	xmlChar *content = NULL, *str_content = NULL;

	if (!xplCheckNodeListForText(cur->children))
		return xplCreateErrorNode(ctxt->parent, BAD_CAST "element '%s' has non-text nodes inside", cur->name);
	if (type == JXE_STRING)
		ADD_DATA("\"", 1);
	content = xmlNodeListGetString(cur->doc, cur->children, 1);
	if (ctxt->value_type_check)
		switch(type)
		{
		case JXE_STRING:
			break;
		case JXE_NUMBER:
			if (!xstrIsNumber(content))
			{
				ret = xplCreateErrorNode(ctxt->parent, BAD_CAST "element '%s' content '%s' is non-numeric", cur->name, content);
				goto done;
			}
			break;
		case JXE_BOOLEAN:
			if (xmlStrcmp(content, BAD_CAST "false") && xmlStrcmp(content, BAD_CAST "true"))
			{
				ret = xplCreateErrorNode(ctxt->parent, BAD_CAST "element '%s' content '%s' is non-boolean", cur->name, content);
				goto done;
			}
			break;
		case JXE_NULL:
			if (content && *content)
			{
				ret = xplCreateErrorNode(ctxt->parent, BAD_CAST "element '%s' content '%s' is non-empty", cur->name, content);
				goto done;
			}
			break;
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
		}
	if (type == JXE_STRING && content)
		str_content = _jsonxEscapeString(content);
	else
		str_content = content;
	if (type == JXE_NULL)
		ADD_DATA(BAD_CAST "null", 4);
	else if (str_content)
		ADD_DATA(str_content, xmlStrlen(str_content));
	if (type == JXE_STRING)
		ADD_DATA("\"", 1);
done:
	if (str_content && str_content != content)
		XPL_FREE(str_content);
	if (content)
		XPL_FREE(content);
	return ret;
}

static xmlNodePtr _jsonxSerializeNode(xmlNodePtr cur, jsonxSerializeCtxtPtr ctxt)
{
	xmlChar *name = NULL, *escaped_name = NULL;
	xmlNodePtr ret = NULL;
	jsonxElementType el_type;

	if (!cur->ns || !_checkJsonXNs(cur->ns, &ctxt->jsonx_ns))
	{
		if (ctxt->strict_tag_names)
			return xplCreateErrorNode(ctxt->parent, BAD_CAST "element '%s:%s' is not in JSONX namespace", cur->ns? cur->ns->prefix: NULL, cur->name);
		return NULL;
	}
	el_type = _getElementType(cur);
	if (el_type == JXE_NONE)
	{
		if (ctxt->strict_tag_names)
			return xplCreateErrorNode(ctxt->parent, BAD_CAST "unknown tag name '%s'", cur->name);
		return NULL;
	}
	if (ctxt->container_type != JXE_NONE)
	{
		if (!ctxt->is_first_item)
			ADD_DATA(",", 1);
		else
			ctxt->is_first_item = false;
	}
	name = xmlGetNoNsProp(cur, BAD_CAST "name");
	if (name)
	{
		if (ctxt->container_type != JXE_OBJECT)
		{
			ret = xplCreateErrorNode(ctxt->parent, BAD_CAST "cannot use named items outside of object");
			goto done;
		}
		ADD_DATA("\"", 1);
		escaped_name = _jsonxEscapeString(name);
		ADD_DATA(escaped_name, xmlStrlen(escaped_name));
		ADD_DATA("\":", 2);
	} else if (ctxt->container_type == JXE_OBJECT) {
		ret = xplCreateErrorNode(ctxt->parent, BAD_CAST "items inside an object must be named");
		goto done;
	}

	if (JSONX_TYPE_IS_CONTAINER(el_type))
		ret = _jsonxSerializeList(cur, ctxt, el_type);
	else
		ret = _jsonxSerializeAtom(cur, ctxt, el_type);
done:
	if (escaped_name)
		xmlFree(escaped_name);
	if (name)
		XPL_FREE(name);
	return ret;
}

static xmlNodePtr _jsonxSerializeNodeList(xmlNodePtr first, jsonxSerializeCtxtPtr ctxt)
{
	xmlNodePtr error;

	while (first)
	{
		if ((error = _jsonxSerializeNode(first, ctxt)))
			return error;
		first = first->next;
	}
	return NULL;
}

xmlNodePtr xplJsonXSerializeNodeList(xmlNodePtr list, bool strictTagNames, bool valueTypeCheck)
{
	jsonxSerializeCtxt ctxt;
	xmlNodePtr ret;

	ctxt.buf = rbCreateBufParams(1024, RB_GROW_DOUBLE, 2);
	if (!ctxt.buf)
		return NULL;
	ctxt.strict_tag_names = strictTagNames;
	ctxt.value_type_check = valueTypeCheck;
	ctxt.jsonx_ns = NULL;
	ctxt.parent = list->parent;
	ctxt.container_type = JXE_NONE;
	ctxt.is_first_item = true;

	if ((ret = _jsonxSerializeNodeList(list, &ctxt)))
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
