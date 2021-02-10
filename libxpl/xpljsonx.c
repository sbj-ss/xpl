#include <stdbool.h>
#include <yajl/yajl_common.h>
#include <yajl/yajl_gen.h>
#include <yajl/yajl_parse.h>
#include <libxpl/xplbuffer.h>
#include <libxpl/xpljsonx.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>

#ifdef _LEAK_DETECTION
static void* _yajlMalloc(void *ctx, size_t sz)
{
	UNUSED_PARAM(ctx);
	return XPL_MALLOC(sz);
}

static void* _yajlRealloc(void *ctx, void *ptr, size_t sz)
{
	UNUSED_PARAM(ctx);
	return XPL_REALLOC(ptr, sz);
}

static void _yajlFree(void *ctx, void *ptr)
{
	UNUSED_PARAM(ctx);
	XPL_FREE(ptr);
}

static const yajl_alloc_funcs yajl_mem_funcs ={
	.malloc = _yajlMalloc,
	.realloc = _yajlRealloc,
	.free = _yajlFree
};

static const yajl_alloc_funcs *yajl_mem_funcs_ptr = &yajl_mem_funcs;
#else
static const yajl_alloc_funcs *yajl_mem_funcs_ptr = NULL;
#endif

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
	yajl_gen gen;
	rbBufPtr buf;
	/* will be changed by serializer */
	xmlNsPtr jsonx_ns;
	jsonxElementType container_type;
	bool out_of_memory;
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

static xmlChar* _yajlGenStatusToString(int status)
{
	switch(status)
	{
	case yajl_gen_status_ok:
		return BAD_CAST "no error";
	case yajl_gen_keys_must_be_strings:
		return BAD_CAST "keys must be strings";
	case yajl_max_depth_exceeded:
		return BAD_CAST "max JSON depth exceeded";
	case yajl_gen_in_error_state:
		return BAD_CAST "yajl_gen_XXX called while already in error state";
	case yajl_gen_generation_complete:
		return BAD_CAST "extra data after the end of JSON document (sequential atoms without a parent?..)";
	case yajl_gen_invalid_number:
		return BAD_CAST "invalid number";
	case yajl_gen_no_buf:
		return BAD_CAST "custom print callback is used so YAJL buffer is empty";
	case yajl_gen_invalid_string:
		return BAD_CAST "invalid UTF-8 string";
	default:
		return BAD_CAST "unknown error";
	}
}

#define CHECK_RESULT_AND_OOM(result, ctxt, allow_complete) \
	do { \
		if ((result != yajl_gen_status_ok) && (!(allow_complete) || result != yajl_gen_generation_complete)) \
		{ \
			ret = xplCreateErrorNode(ctxt->parent, _yajlGenStatusToString(result)); \
			goto done; \
		} \
		if (ctxt->out_of_memory) \
		{ \
			ret = xplCreateErrorNode(ctxt->parent, BAD_CAST "%s(): out of memory", __FUNCTION__); \
			goto done; \
		} \
	} while(0) \

static xmlNodePtr _jsonxSerializeNodeList(xmlNodePtr first, jsonxSerializeCtxtPtr ctxt);

static xmlNodePtr _jsonxSerializeList(xmlNodePtr cur, jsonxSerializeCtxtPtr ctxt, jsonxElementType containerType)
{
	jsonxElementType prev_container_type;
	xmlNodePtr ret = NULL;
	int yajl_result;

	if (containerType == JXE_ARRAY)
		yajl_result = yajl_gen_array_open(ctxt->gen);
	else if (containerType == JXE_OBJECT)
		yajl_result = yajl_gen_map_open(ctxt->gen);
	else {
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return xplCreateErrorNode(ctxt->parent, BAD_CAST "internal error");
	}
	CHECK_RESULT_AND_OOM(yajl_result, ctxt, false);
	prev_container_type = ctxt->container_type;
	ctxt->container_type = containerType;
	if ((ret = _jsonxSerializeNodeList(cur->children, ctxt)))
		goto done;
	if (containerType == JXE_ARRAY)
		yajl_result = yajl_gen_array_close(ctxt->gen);
	else if (containerType == JXE_OBJECT)
		yajl_result = yajl_gen_map_close(ctxt->gen);
	CHECK_RESULT_AND_OOM(yajl_result, ctxt, prev_container_type == JXE_NONE);
done:
	ctxt->container_type = prev_container_type;
	return ret;
}

static xmlNodePtr _jsonxSerializeAtom(xmlNodePtr cur, jsonxSerializeCtxtPtr ctxt, jsonxElementType type)
{
	xmlNodePtr ret = NULL;
	xmlChar *content = NULL;
	int yajl_result;

	if (!xplCheckNodeListForText(cur->children))
		return xplCreateErrorNode(ctxt->parent, BAD_CAST "element '%s' has non-text nodes inside", cur->name);
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
	switch(type)
	{
	case JXE_STRING:
		yajl_result = yajl_gen_string(ctxt->gen, content, xmlStrlen(content));
		break;
	case JXE_NUMBER:
		yajl_result = yajl_gen_number(ctxt->gen, (char*) content, xmlStrlen(content));
		break;
	case JXE_BOOLEAN:
		yajl_result = yajl_gen_bool(ctxt->gen, content && content[0] == 't');
		break;
	case JXE_NULL:
		yajl_result = yajl_gen_null(ctxt->gen);
		break;
	default:
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		ret = xplCreateErrorNode(ctxt->parent, BAD_CAST "internal error");
		goto done;
	}
	CHECK_RESULT_AND_OOM(yajl_result, ctxt, false); // ??
done:
	if (content)
		XPL_FREE(content);
	return ret;
}

static xmlNodePtr _jsonxSerializeNode(xmlNodePtr cur, jsonxSerializeCtxtPtr ctxt)
{
	xmlChar *name = NULL;
	xmlNodePtr ret = NULL;
	jsonxElementType el_type;
	int yajl_result;

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
	name = xmlGetNoNsProp(cur, BAD_CAST "name");
	if (name)
	{
		if (ctxt->container_type != JXE_OBJECT)
		{
			ret = xplCreateErrorNode(ctxt->parent, BAD_CAST "cannot use named items outside of object");
			goto done;
		}
	} else if (ctxt->container_type == JXE_OBJECT) {
		ret = xplCreateErrorNode(ctxt->parent, BAD_CAST "items inside an object must be named");
		goto done;
	}
	if (name)
	{
		yajl_result = yajl_gen_string(ctxt->gen, name, xmlStrlen(name));
		CHECK_RESULT_AND_OOM(yajl_result, ctxt, ctxt->container_type != JXE_NONE);
	}
	if (JSONX_TYPE_IS_CONTAINER(el_type))
		ret = _jsonxSerializeList(cur, ctxt, el_type);
	else
		ret = _jsonxSerializeAtom(cur, ctxt, el_type);
done:
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

static void _yajlPrint(void *ctx, const char *str, size_t len)
{
	jsonxSerializeCtxtPtr jctxt = (jsonxSerializeCtxtPtr) ctx;

	if (!str)
		return;
	if (rbAddDataToBuf(jctxt->buf, str, len) != RB_RESULT_OK)
		jctxt->out_of_memory = true;
}

xmlNodePtr xplJsonXSerializeNodeList(xmlNodePtr list, bool strictTagNames, bool valueTypeCheck, bool prettyPrint)
{
	jsonxSerializeCtxt ctxt;
	xmlNodePtr ret;

	if (!(ctxt.buf = rbCreateBufParams(2048, RB_GROW_DOUBLE, 0)))
	{
		ret = xplCreateErrorNode(list->parent, BAD_CAST "%s(): rbCreateBufParams() failed", __FUNCTION__);
		goto done;
	}
	if (!(ctxt.gen = yajl_gen_alloc(yajl_mem_funcs_ptr)))
	{
		ret = xplCreateErrorNode(list->parent, BAD_CAST "%s(): yajl_gen_alloc() failed", __FUNCTION__);
		goto done;
	}
	if (!yajl_gen_config(ctxt.gen, yajl_gen_print_callback, _yajlPrint, &ctxt))
	{
		ret = xplCreateErrorNode(list->parent, BAD_CAST "%s(): yajl_gen_config(yajl_gen_print_callback) failed", __FUNCTION__);
		goto done;
	}
	if (!yajl_gen_config(ctxt.gen, yajl_gen_beautify, (int) prettyPrint))
	{
		ret = xplCreateErrorNode(list->parent, BAD_CAST "%s(): yajl_gen_config(yajl_gen_beautify) failed", __FUNCTION__);
		goto done;
	}
	ctxt.strict_tag_names = strictTagNames;
	ctxt.value_type_check = valueTypeCheck;
	ctxt.jsonx_ns = NULL;
	ctxt.parent = list->parent;
	ctxt.container_type = JXE_NONE;
	ctxt.out_of_memory = false;

	if ((ret = _jsonxSerializeNodeList(list, &ctxt)))
		goto done;
	if (rbAddDataToBuf(ctxt.buf, "", 1) != RB_RESULT_OK) // null-terminate
	{
		ret = xplCreateErrorNode(list->parent, BAD_CAST "%s(): out of memory", __FUNCTION__);
		goto done;
	}

	ret = xmlNewDocText(list->doc, NULL);
	ret->content = rbDetachBufContent(ctxt.buf);
done:
	if (ctxt.buf)
		rbFreeBuf(ctxt.buf);
	if (ctxt.gen)
		yajl_gen_free(ctxt.gen);
	return ret;
}
