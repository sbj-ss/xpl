#include <libxpl/xplbuffer.h>
#include <libxpl/xplcommand.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>

void xplCmdXJsonSerializeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

typedef struct _xplCmdXJsonSerializeParams
{
	bool force_quotes;
	bool strict_tag_names;
	bool value_type_check;
	bool single_quotes;
} xplCmdXJsonSerializeParams, *xplCmdXJsonSerializeParamsPtr;

static const xplCmdXJsonSerializeParams params_stencil =
{
	.force_quotes = false,
	.strict_tag_names = false,
	.value_type_check = false,
	.single_quotes = false
};

xplCommand xplXJsonSerializeCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdXJsonSerializeEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdXJsonSerializeParams),
	.parameters = {
		{
			.name = BAD_CAST "forcequotes",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.force_quotes
		}, {
			.name = BAD_CAST "stricttagnames",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.strict_tag_names
		}, {
			.name = BAD_CAST "valuetypecheck",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.value_type_check
		}, {
			.name = BAD_CAST "singlequotes",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.single_quotes
		}, {
			.name = NULL
		}
	}
};

#define XJSON_SCHEMA_URI BAD_CAST "http://www.ibm.com/xmlns/prod/2009/jsonx"

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

typedef enum _xjsonElementType {
	XJE_ARRAY,
	XJE_OBJECT,
	XJE_STRING,
	XJE_NUMBER,
	XJE_BOOLEAN,
	XJE_NULL,
	XJE_NONE
} xjsonElementType;

#define TYPE_IS_CONTAINER(t) ((t) == XJE_OBJECT || (t) == XJE_ARRAY)

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

typedef struct _xjsonSerializeCtxt {
	xmlNodePtr command_element;
	xplCmdXJsonSerializeParamsPtr params;
	rbBufPtr buf;
	/* will be changed by serializer */
	xmlNsPtr ns;
	xjsonElementType container_type;
	bool is_first_item;
} xjsonSerializeCtxt, *xjsonSerializeCtxtPtr;

static xmlNodePtr _createMemoryError(xjsonSerializeCtxtPtr ctxt)
{
	return xplCreateErrorNode(ctxt->command_element, BAD_CAST "insufficient memory");
}

#define ADD_DATA(d, size) \
	do { \
		if (rbAddDataToBuf(ctxt->buf, d, size) != RB_RESULT_OK) \
		{ \
			ret = _createMemoryError(ctxt); \
			goto done; \
		} \
	} while (0)

/* forward declarations */
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
		return xplCreateErrorNode(ctxt->command_element, BAD_CAST "element '%s' has non-text nodes inside", cur->name);
	if ((type == XJE_STRING) || (ctxt->params->force_quotes && (type != XJE_NULL)))
		ADD_DATA(ctxt->params->single_quotes? "'": "\"", 1);
	content = xmlNodeListGetString(cur->doc, cur->children, 1);
	if (ctxt->params->value_type_check)
		switch(type)
		{
		case XJE_STRING:
			break;
		case XJE_NUMBER:
			if (!xstrIsNumber(content))
			{
				ret = xplCreateErrorNode(ctxt->command_element, BAD_CAST "element '%s' content '%s' is non-numeric", cur->name, content);
				goto done;
			}
			break;
		case XJE_BOOLEAN:
			if (xmlStrcmp(content, BAD_CAST "false") && xmlStrcmp(content, BAD_CAST "true"))
			{
				ret = xplCreateErrorNode(ctxt->command_element, BAD_CAST "element '%s' content '%s' is non-boolean", cur->name, content);
				goto done;
			}
			break;
		case XJE_NULL:
			if (content && *content)
			{
				ret = xplCreateErrorNode(ctxt->command_element, BAD_CAST "element '%s' content '%s' is non-empty", cur->name, content);
				goto done;
			}
			break;
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
		}
	ADD_DATA(type == XJE_NULL? BAD_CAST "null": content, xmlStrlen(type == XJE_NULL? BAD_CAST "null": content));
	if ((type == XJE_STRING) || (ctxt->params->force_quotes && (type != XJE_NULL)))
		ADD_DATA(ctxt->params->single_quotes? "'": "\"", 1);
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

	if (!cur->ns || !_checkJsonNs(cur->ns, &ctxt->ns))
	{
		if (ctxt->params->strict_tag_names)
			return xplCreateErrorNode(ctxt->command_element, BAD_CAST "element '%s:%s' is not in XJSON namespace", cur->ns? cur->ns->prefix: NULL, cur->name);
		return NULL;
	}
	el_type = _getElementType(cur);
	if (el_type == XJE_NONE)
	{
		if (ctxt->params->strict_tag_names)
			return xplCreateErrorNode(ctxt->command_element, BAD_CAST "unknown tag name '%s'", cur->name);
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
			ret = xplCreateErrorNode(ctxt->command_element, BAD_CAST "cannot use named items outside of object");
			goto done;
		}
		ADD_DATA(ctxt->params->single_quotes? "'": "\"", 1);
		ADD_DATA(name, xmlStrlen(name));
		ADD_DATA(ctxt->params->single_quotes? "'": "\"", 1);
		ADD_DATA(":", 1);
	} else if (ctxt->container_type == XJE_OBJECT) {
		ret = xplCreateErrorNode(ctxt->command_element, BAD_CAST "items inside an object must be named");
		goto done;
	}

	if (TYPE_IS_CONTAINER(el_type))
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

void xplCmdXJsonSerializeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xjsonSerializeCtxt ctxt;
	xmlNodePtr error = NULL, ret;

	ctxt.params = (xplCmdXJsonSerializeParamsPtr) commandInfo->params;
	ctxt.buf = rbCreateBufParams(1024, RB_GROW_DOUBLE, 2);
	if (!ctxt.buf)
	{
		/* no memory for anything, xplCreateErrorNode() will likely fail, too */ // TODO
		ASSIGN_RESULT(NULL, false, true);
		goto done;
	}
	ctxt.ns = NULL;
	ctxt.command_element = commandInfo->element;
	ctxt.container_type = XJE_NONE;
	ctxt.is_first_item = true;
	if ((error = _xjsonSerializeNodeList(commandInfo->element->children, &ctxt)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if (rbAddDataToBuf(ctxt.buf, "", 1) != RB_RESULT_OK) // zero-terminate
	{
		ASSIGN_RESULT(_createMemoryError(&ctxt), true, true);
		goto done;
	}
	ret = xmlNewDocText(commandInfo->element->doc, NULL);
	ret->content = rbDetachBufContent(ctxt.buf);
	ASSIGN_RESULT(ret, false, true);
done:
	if (ctxt.buf)
		rbFreeBuf(ctxt.buf);
}
