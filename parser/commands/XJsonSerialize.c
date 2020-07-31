#include <libxpl/xplbuffer.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>
#include "commands/XJsonSerialize.h"

#define XJSON_SCHEMA_URI BAD_CAST "http://www.ibm.com/xmlns/prod/2009/jsonx"

void xplCmdXJsonSerializePrologue(xplCommandInfoPtr commandInfo)
{
}

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

typedef enum _xjsonAtomType {
	XJA_STRING,
	XJA_NUMBER,
	XJA_BOOLEAN,
	XJA_NULL
} xjsonAtomType;

typedef enum _xjsonContainerType {
	XJC_NONE,
	XJC_ARRAY,
	XJC_OBJECT
} xjsonContainerType;

typedef struct _xjsonSerializeCtxt {
	xmlNodePtr command_element;
	rbBufPtr buf;
	bool force_quotes;
	bool strict_tag_names;
	bool value_type_check;
	bool single_quotes;
	/* will be changed by serializer */
	xmlNsPtr ns;
	xjsonContainerType container_type;
	bool is_first_item;
} xjsonSerializeCtxt, *xjsonSerializeCtxtPtr;

static xmlNodePtr _createMemoryError(xjsonSerializeCtxtPtr ctxt)
{
	return xplCreateErrorNode(ctxt->command_element, BAD_CAST "insufficient memory");
}

/* forward declarations */
static xmlNodePtr _xjsonSerializeNodeList(xmlNodePtr first, xjsonSerializeCtxtPtr ctxt);

static xmlNodePtr _xjsonSerializeList(xmlNodePtr cur, xjsonSerializeCtxtPtr ctxt, xjsonContainerType containerType)
{
	bool prev_container_type;
	bool prev_is_first_item;
	xmlNodePtr ret = NULL;

	if (rbAddDataToBuf(ctxt->buf, (containerType == XJC_ARRAY)? "[": "{", 1) != RB_RESULT_OK)
		return _createMemoryError(ctxt);
	prev_container_type = ctxt->container_type;
	prev_is_first_item = ctxt->is_first_item;
	ctxt->container_type = containerType;
	ctxt->is_first_item = true;
	if ((ret = _xjsonSerializeNodeList(cur->children, ctxt)))
		goto done;
	if (rbAddDataToBuf(ctxt->buf, (containerType == XJC_ARRAY)? "]": "}", 1) != RB_RESULT_OK)
		ret = _createMemoryError(ctxt);
done:
	ctxt->container_type = prev_container_type;
	ctxt->is_first_item = prev_is_first_item;
	return ret;
}

static xmlNodePtr _xjsonSerializeAtom(xmlNodePtr cur, xjsonSerializeCtxtPtr ctxt, xjsonAtomType type)
{
	xmlNodePtr ret = NULL;
	xmlChar *content = NULL;

	if (!checkNodeListForText(cur->children))
		return xplCreateErrorNode(ctxt->command_element, BAD_CAST "element \"%s\" has non-text nodes inside", cur->name);
	if ((type == XJA_STRING) || (ctxt->force_quotes && (type != XJA_NULL)))
	{
		if (rbAddDataToBuf(ctxt->buf, ctxt->single_quotes? "'": "\"", 1) != RB_RESULT_OK)
			return _createMemoryError(ctxt);
	}
	content = xmlNodeListGetString(cur->doc, cur->children, 1);
	switch(type)
	{
	case XJA_STRING:
		break;
	case XJA_NUMBER:
		if (ctxt->value_type_check && !xstrIsNumber(content))
		{
			ret = xplCreateErrorNode(ctxt->command_element, BAD_CAST "element \"%s\" content \"%s\" is non-numeric", cur->name, content);
			goto done;
		}
		break;
	case XJA_BOOLEAN:
		if (ctxt->value_type_check && xmlStrcmp(content, BAD_CAST "false") && xmlStrcmp(content, BAD_CAST "true"))
		{
			ret = xplCreateErrorNode(ctxt->command_element, BAD_CAST "element \"%s\" content \"%s\" is non-boolean", cur->name, content);
			goto done;
		}
		break;
	case XJA_NULL:
		if (ctxt->value_type_check && content && *content)
		{
			ret = xplCreateErrorNode(ctxt->command_element, BAD_CAST "element \"%s\" content \"%s\" is non-empty", cur->name, content);
			goto done;
		}
		break;
	default:
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	}
	if (rbAddDataToBuf(ctxt->buf, (type == XJA_NULL)? BAD_CAST "null": content, xmlStrlen((type == XJA_NULL)? BAD_CAST "null": content)) != RB_RESULT_OK)
	{
		ret = _createMemoryError(ctxt);
		goto done;
	}
	if ((type == XJA_STRING) || (ctxt->force_quotes && (type != XJA_NULL)))
	{
		if (rbAddDataToBuf(ctxt->buf, ctxt->single_quotes? "'": "\"", 1) != RB_RESULT_OK)
			return _createMemoryError(ctxt);
	}
done:
	if (content) XPL_FREE(content);
	return ret;
}

static xmlNodePtr _xjsonSerializeNode(xmlNodePtr cur, xjsonSerializeCtxtPtr ctxt)
{
	xmlChar *name = NULL;
	xmlNodePtr ret = NULL;

	if (!cur->ns || !_checkJsonNs(cur->ns, &(ctxt->ns)))
	{
		if (ctxt->strict_tag_names)
			return xplCreateErrorNode(ctxt->command_element, BAD_CAST "element \"%s:%s\" is not in XJSON namespace", cur->ns? cur->ns->prefix: NULL, cur->name);
		return NULL;
	}
	if (ctxt->container_type != XJC_NONE)
	{
		if (!ctxt->is_first_item)
		{
			if (rbAddDataToBuf(ctxt->buf, ",", 1 != RB_RESULT_OK))
				return _createMemoryError(ctxt);
		} else
			ctxt->is_first_item = false;
	}
	name = xmlGetNoNsProp(cur, BAD_CAST "name");
	if (name)
	{
		if (ctxt->container_type != XJC_OBJECT)
		{
			ret = xplCreateErrorNode(ctxt->command_element, BAD_CAST "cannot use named items outside of object");
			goto done;
		}
		if (rbAddDataToBuf(ctxt->buf, name, xmlStrlen(name)) != RB_RESULT_OK)
		{
			ret = _createMemoryError(ctxt);
			goto done;
		}
		if (rbAddDataToBuf(ctxt->buf, ":", 1) != RB_RESULT_OK)
		{
			ret = _createMemoryError(ctxt);
			goto done;
		}
	} else if (ctxt->container_type == XJC_OBJECT) {
		ret = xplCreateErrorNode(ctxt->command_element, BAD_CAST "items inside an object must be named");
		goto done;
	}

	if (!xmlStrcmp(cur->name, BAD_CAST "array"))
		ret = _xjsonSerializeList(cur, ctxt, XJC_ARRAY);
	else if (!xmlStrcmp(cur->name, BAD_CAST "object")) 
		ret = _xjsonSerializeList(cur, ctxt, XJC_OBJECT);
	else if (!xmlStrcmp(cur->name, BAD_CAST "string")) 
		ret = _xjsonSerializeAtom(cur, ctxt, XJA_STRING);
	else if (!xmlStrcmp(cur->name, BAD_CAST "number")) 
		ret = _xjsonSerializeAtom(cur, ctxt, XJA_NUMBER);
	else if (!xmlStrcmp(cur->name, BAD_CAST "boolean")) 
		ret = _xjsonSerializeAtom(cur, ctxt, XJA_BOOLEAN);
	else if (!xmlStrcmp(cur->name, BAD_CAST "null")) 
		ret = _xjsonSerializeAtom(cur, ctxt, XJA_NULL);
	else if (ctxt->strict_tag_names)
		ret = xplCreateErrorNode(ctxt->command_element, BAD_CAST "unknown tag name: \"%s\"", cur->name);
done:
	if (name) XPL_FREE(name);
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

void xplCmdXJsonSerializeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define FORCE_QUOTES_ATTR (BAD_CAST "forcequotes")
#define STRICT_TAG_NAMES_ATTR (BAD_CAST "stricttagnames")
#define VALUE_TYPE_CHECK_ATTR (BAD_CAST "valuetypecheck")
#define SINGLE_QUOTES_ATTR (BAD_CAST "singlequotes")
	xjsonSerializeCtxt ctxt;
	xmlNodePtr error = NULL, ret;

	ctxt.buf = NULL;
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, FORCE_QUOTES_ATTR, &ctxt.force_quotes, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, STRICT_TAG_NAMES_ATTR, &ctxt.strict_tag_names, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, VALUE_TYPE_CHECK_ATTR, &ctxt.value_type_check, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, SINGLE_QUOTES_ATTR, &ctxt.single_quotes, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}

	ctxt.buf = rbCreateBufParams(1024, RB_GROW_DOUBLE, 2);
	if (!ctxt.buf)
	{
		/* no memory for anything, xplCreateErrorNode() will likely fail, too */
		ASSIGN_RESULT(NULL, false, true);
		goto done;
	}
	ctxt.ns = NULL;
	ctxt.command_element = commandInfo->element;
	ctxt.container_type = XJC_NONE;
	if ((error = _xjsonSerializeNodeList(commandInfo->element->children, &ctxt)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if (rbAddDataToBuf(ctxt.buf, "", 1) != RB_RESULT_OK)
	{
		ASSIGN_RESULT(_createMemoryError(&ctxt), true, true);
		goto done;
	}
	ret = xmlNewDocText(commandInfo->element->doc, NULL);
	ret->content = rbDetachBufContent(ctxt.buf);
	ASSIGN_RESULT(ret, false, true);
done:
	if (ctxt.buf) rbFreeBuf(ctxt.buf);
}

xplCommand xplXJsonSerializeCommand = { xplCmdXJsonSerializePrologue, xplCmdXJsonSerializeEpilogue };
