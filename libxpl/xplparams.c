#include <ctype.h>
#include <string.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplparams.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>

xplExpectType xplExpectTypeFromString(const xmlChar *expect)
{
	if (!expect)
		return XPL_EXPECT_UNSPECIFIED;
	if (!xmlStrcasecmp(expect, BAD_CAST "number"))
		return XPL_EXPECT_NUMBER;
	if (!xmlStrcasecmp(expect, BAD_CAST "hex"))
		return XPL_EXPECT_HEX;
	if (!xmlStrcasecmp(expect, BAD_CAST "string"))
		return XPL_EXPECT_STRING;
	if (!xmlStrcasecmp(expect, BAD_CAST "path"))
		return XPL_EXPECT_PATH;
	if (!xmlStrcasecmp(expect, BAD_CAST "any"))
		return XPL_EXPECT_ANY;
	return XPL_EXPECT_UNKNOWN;
}

xmlChar* xplExpectTypeGetter(xplCommandInfoPtr commandInfo, const xmlChar *expect, int *result)
{
	UNUSED_PARAM(commandInfo);
	if (!result)
		return BAD_CAST XPL_STRDUP("xplExpectTypeGetter: result is NULL");
	*result = xplExpectTypeFromString(expect);
	if (*result == XPL_EXPECT_UNKNOWN)
		return xplFormatMessage(BAD_CAST "unknown expect type '%s'", expect);
	return NULL;
}

static void xplCleanTextValueInner(xmlChar *data_buf, xplExpectType expect, xmlChar *out)
{
	bool dot = 0;
	xmlChar *p = data_buf;
	bool leading_zero = false, leading_x = false;

	if (!p)
	{
		*out = 0;
		return;
	}
	while (*p)
	{
		switch (expect)
		{
			case XPL_EXPECT_NUMBER:
				if ((p == data_buf) && (*p == '-'))
					*out++ = *p;
				else if (('0' <= *p) && (*p <= '9'))
					*out++ = *p;
				else if (*p == '.') {
					if (!dot)
					{
						*out++ = *p;
						dot = true;
					}
				}
				break;
			case XPL_EXPECT_HEX: /* 0[xX][0-9a-fA-F]+ */
				if ((p == data_buf) && (*p == '0'))
				{
					leading_zero = true;
					*out++ = *p;
				} else if (leading_zero && (p == data_buf + 1) && ((*p == 'x') || (*p == 'X'))) {
					leading_x = true;
					*out++ = *p;
				/* could use a lookup table here but too lazy */
				} else if (leading_x && ((('0' <= *p) && (*p <= '9')) || (('a' <= *p) && (*p <= 'f')) || (('A' <= *p) && (*p <= 'F'))))
					*out++ = *p;
				break;
			case XPL_EXPECT_STRING:
				if (*p != '\'')
					*out++ = *p;
				else
					*out++ = '`';
				break;
			case XPL_EXPECT_PATH:
				if ((p == data_buf) && ((*p == XPR_PATH_DELIM) || (*p == XPR_PATH_INVERSE_DELIM)))
					NOOP(); /* skip absolute paths */
				else if (dot && (*p == '.'))
					NOOP(); /* skip multiple dots */
				else if (*p == '.') {
					*out++ = *p;
					dot = true;
				} else {
					*out++ = *p;
					dot = false;
				}
				break;
			default:
				*out++ = *p;
				break;
		}
		p++;
	}
	*out = 0;
}

xmlChar* xplCleanTextValue(xmlChar *data_buf, xplExpectType expect)
{
	xmlChar *ret;

	if (!data_buf)
		return NULL;
	ret = (xmlChar*) XPL_MALLOC(xmlStrlen(data_buf) + 1);
	if (!ret)
		return NULL;
	xplCleanTextValueInner(data_buf, expect, ret);
	return ret;
}

int xplParamTypeMaskFromString(const xmlChar* mask)
{
	char *token, *state;
	int ret = 0;

	if (!mask)
		return 0;
	token = strtok_r((char*) mask, ", ", &state);
	while (token)
	{
		if (!strcmp(token, "userdata"))
			ret |= XPL_PARAM_TYPE_USERDATA;
		else if (!strcmp(token, "file"))
			ret |= XPL_PARAM_TYPE_FILE;
		else if (!strcmp(token, "header"))
			ret |= XPL_PARAM_TYPE_HEADER;
		else if (!strcmp(token, "any"))
			ret |= 0xFF;
		else
			return -1;
		token = strtok_r(NULL, ", ", &state);
	}
	return ret;
}

xmlChar* xplParamTypeMaskGetter(xplCommandInfoPtr info, const xmlChar *mask, int *result)
{
	UNUSED_PARAM(info);
	if ((*result = xplParamTypeMaskFromString(mask)) == -1)
		return xplFormatMessage(BAD_CAST "invalid type mask '%s'", mask);
	return NULL;
}

bool xplParamTypeIsAtomic(xplParamType type)
{
	switch(type)
	{
	case XPL_PARAM_TYPE_USERDATA:
	case XPL_PARAM_TYPE_HEADER:
		return true;
	default:
		return false;
	}
}

/* xplParamFileInfo */
xplParamFileInfoPtr xplParamFileInfoCreate(xmlChar *realPath, xmlChar *filename, int Size)
{
	xplParamFileInfoPtr ret;

	ret = (xplParamFileInfoPtr) XPL_MALLOC(sizeof(xplParamFileInfo));
	if (!ret)
		return NULL;
	ret->real_path = realPath;
	ret->filename = filename;
	ret->size = Size;
	return ret;
}

xplParamFileInfoPtr xplParamFileInfoCopy(const xplParamFileInfoPtr src)
{
	if (!src)
		return NULL;
	return xplParamFileInfoCreate(
		BAD_CAST XPL_STRDUP((char*) src->real_path),
		BAD_CAST XPL_STRDUP((char*) src->filename),
		src->size
	);
}

void xplParamFileInfoFree(xplParamFileInfoPtr info)
{
	if (!info)
		return;
	if (info->filename)
		XPL_FREE(info->filename);
	if (info->real_path)
		XPL_FREE(info->real_path);
	XPL_FREE(info);
}

/* xplParamValues */
#define INITIAL_VALUES_SIZE 2

static xplParamValuesPtr xplParamValuesCreateInner(int size)
{
	xplParamValuesPtr ret = (xplParamValuesPtr) XPL_MALLOC(sizeof(xplParamValues));
	if (!ret)
		return NULL;
	ret->param_tab = (void**) XPL_MALLOC(sizeof(void*) * size);
	if (ret->param_tab)
		ret->param_max = size;
	else
		ret->param_max = 0;
	ret->param_nr = 0;
	ret->is_locked = false;
	ret->type = XPL_PARAM_TYPE_EMPTY;
	return ret;
}

xplParamValuesPtr xplParamValuesCreate()
{
	return xplParamValuesCreateInner(INITIAL_VALUES_SIZE);
}

xplParamValuesPtr xplParamValuesCopy(const xplParamValuesPtr src)
{
	xplParamValuesPtr ret;
	int i;

	if (!src)
		return NULL;
	ret = xplParamValuesCreateInner(src->param_max);
	if (!ret)
		return NULL;
	for (i = 0; i < src->param_nr; i++)
	{
		if (xplParamTypeIsAtomic(src->type))
			ret->param_tab[i] = XPL_STRDUP(src->param_tab[i]);
		else if (src->type == XPL_PARAM_TYPE_FILE)
			ret->param_tab[i] = xplParamFileInfoCopy((const xplParamFileInfoPtr) src->param_tab[i]);
		else {
			/* we missed something */
			ret->param_tab[i] = NULL;
		}
	}
	ret->param_nr = src->param_nr;
	ret->type = src->type;
	return ret;
}

static xplParamResult xplParamValuesAddSomething(xplParamValuesPtr values, void *value)
{
	if (values->is_locked)
		return XPL_PARAM_RES_READ_ONLY;
	if (values->param_nr == values->param_max)
	{
		if (values->param_tab)
		{
			values->param_max *= 2;
			values->param_tab = (void**) XPL_REALLOC(values->param_tab, values->param_max * sizeof(void*));
		} else {
			values->param_tab = (void**) XPL_MALLOC(INITIAL_VALUES_SIZE * sizeof(void*));
			values->param_max = INITIAL_VALUES_SIZE;
		}
		if (!values->param_tab)
		{
			values->param_nr = values->param_max = 0;
			return XPL_PARAM_RES_OUT_OF_MEMORY;
		}
	}
	values->param_tab[values->param_nr++] = value;
	return XPL_PARAM_RES_OK;
}

xplParamResult xplParamValuesAdd(xplParamValuesPtr values, xmlChar *value, xplParamType type)
{
	xplParamResult ret; 

	if (!values)
		return XPL_PARAM_RES_INVALID_INPUT;
	if ((values->type != XPL_PARAM_TYPE_EMPTY) && 
		(!xplParamTypeIsAtomic(values->type) || (values->type != type)))
		return XPL_PARAM_RES_TYPE_CLASH;
	ret = xplParamValuesAddSomething(values, value);
	if (ret != XPL_PARAM_RES_OK)
		return ret;
	values->type = type;
	return XPL_PARAM_RES_OK;
}

xplParamResult xplParamValuesAddFileInfo(xplParamValuesPtr values, xmlChar *real_path, xmlChar *filename, int size)
{
	xplParamResult ret; 
	xplParamFileInfoPtr info;

	if (!values)
		return XPL_PARAM_RES_INVALID_INPUT;
	if ((values->type != XPL_PARAM_TYPE_EMPTY) && (values->type != XPL_PARAM_TYPE_FILE))
		return XPL_PARAM_RES_TYPE_CLASH;
	info = xplParamFileInfoCreate(real_path, filename, size);
	if (!info)
		return XPL_PARAM_RES_OUT_OF_MEMORY;
	ret = xplParamValuesAddSomething(values, info);
	if (ret != XPL_PARAM_RES_OK)
		return ret;
	values->type = XPL_PARAM_TYPE_FILE;
	return XPL_PARAM_RES_OK;
}

xplParamResult xplParamValuesReplace(xplParamValuesPtr values, xmlChar *value, xplParamType type)
{
	int i;

	if (!values)
		return XPL_PARAM_RES_INVALID_INPUT;
	if (values->is_locked)
		return XPL_PARAM_RES_READ_ONLY;
	if ((values->type != XPL_PARAM_TYPE_EMPTY) && 
		(!xplParamTypeIsAtomic(values->type) || (values->type != type)))
		return XPL_PARAM_RES_TYPE_CLASH;
	if (values->param_tab)
	{
		for (i = 0; i < values->param_nr; i++)
			XPL_FREE(values->param_tab[i]);
		values->param_tab = (void**) XPL_REALLOC(values->param_tab, INITIAL_VALUES_SIZE * sizeof(xmlChar*));
	} else
		values->param_tab = (void**) XPL_MALLOC(INITIAL_VALUES_SIZE * sizeof(xmlChar*));
	if (!values->param_tab)
	{
		values->param_nr = values->param_max = 0;
		return XPL_PARAM_RES_OUT_OF_MEMORY;
	}
	values->param_max = INITIAL_VALUES_SIZE;
	values->param_nr = 1;
	values->param_tab[0] = value;
	values->type = type;
	return XPL_PARAM_RES_OK;
}

void xplParamValuesFree(xplParamValuesPtr values)
{
	int i;

	if (!values)
		return;
	if (values->param_tab)
	{
		for (i = 0; i < values->param_nr; i++)
		{
			if (xplParamTypeIsAtomic(values->type))
			{
				if (values->param_tab[i])
					XPL_FREE(values->param_tab[i]);
			} else if ((values->type == XPL_PARAM_TYPE_FILE) && values->param_tab[i])
				xplParamFileInfoFree((xplParamFileInfoPtr) values->param_tab[i]);
			else
				DISPLAY_INTERNAL_ERROR_MESSAGE();
		}
		XPL_FREE(values->param_tab);
	}
	XPL_FREE(values);
}

xmlChar* xplParamValuesToString(const xplParamValuesPtr values, bool unique, const xmlChar *delim, xplExpectType expect)
{
	xmlChar *ret, *cur, *tail = NULL;
	int delim_len;
	int total_len = 0;
	int i;
	xmlHashTablePtr unique_table = NULL;

	if (!values)
		return NULL;
	if (!xplParamTypeIsAtomic(values->type))
		return NULL; /* TODO: should we serialize complex params? */
	delim_len = delim? xmlStrlen(delim): 0;
	for (i = 0; i < values->param_nr; i++)
		if (values->param_tab[i])
			total_len += xmlStrlen((const xmlChar*) values->param_tab[i]);
	total_len += (values->param_nr - 1)*delim_len;
	if (!total_len)
		return NULL;
	cur = ret = (xmlChar*) XPL_MALLOC(total_len + 1);
	if (!ret)
		return NULL;
	if (unique)
		unique_table = xmlHashCreate(values->param_nr);
	for (i = 0; i < values->param_nr; i++)
	{
		xplCleanTextValueInner((xmlChar*) values->param_tab[i], expect, cur);
		if (!unique_table || !xmlHashLookup(unique_table, cur))
		{
			if (unique_table)
				xmlHashAddEntry(unique_table, cur, (void*) 1);
			cur += xmlStrlen(cur);
			tail = cur;
			if (delim_len)
			{
				memcpy(cur, delim, delim_len);
				cur += delim_len;
			}
		}
	}
	if (tail)
		*tail = 0;
	if (unique_table)
		xmlHashFree(unique_table, NULL);
	*cur = 0;
	return ret;
}

xmlNodePtr xplParamValuesToList(const xplParamValuesPtr values, bool unique, xplExpectType expect, const xplQName qname, xmlNodePtr parent)
{
	xmlNodePtr ret = NULL, tail, cur, txt;
	xmlHashTablePtr unique_table = NULL;
	xplParamFileInfoPtr file_info;
	xmlChar *clean_value;
	int i;
	char size_buf[32];

	if (!values)
		return NULL;
	if (unique)
	{
		unique_table = xmlHashCreate(values->param_nr);
		if (!unique_table)
			return NULL;
	}
	for (i = 0; i < values->param_nr; i++)
	{
		cur = NULL;
		if (xplParamTypeIsAtomic(values->type))
		{
			clean_value = xplCleanTextValue((xmlChar*) values->param_tab[i], expect);
			if (unique_table && (xmlHashAddEntry(unique_table, clean_value, (void*) 1) == -1))
				XPL_FREE(clean_value); /* duplicate found */
			else {
				cur = xmlNewDocNode(parent->doc, qname.ns, qname.ncname, NULL);
				txt = xmlNewDocText(parent->doc, NULL);
				txt->content = clean_value;
				txt->parent = cur;
				cur->children = cur->last = txt;
			}
		} else if (values->type == XPL_PARAM_TYPE_FILE) {
			cur = xmlNewDocNode(parent->doc, qname.ns, qname.ncname, NULL);
			file_info = (xplParamFileInfoPtr) values->param_tab[i];
			xmlNewProp(cur, BAD_CAST "realpath", file_info->real_path);
			xmlNewProp(cur, BAD_CAST "filename", file_info->filename);
			snprintf(size_buf, sizeof(size_buf), "%d", file_info->size);
			xmlNewProp(cur, BAD_CAST "size", BAD_CAST size_buf);
		} else
			DISPLAY_INTERNAL_ERROR_MESSAGE();
		if (cur)
		{
			if (!ret)
				ret = tail = cur;
			else {
				tail->next = cur;
				cur->prev = tail;
				tail = cur;
			}			
		}
	}
	if (unique_table)
		xmlHashFree(unique_table, NULL);
	return ret;
}

/* Params collection */
xplParamsPtr xplParamsCreate()
{
	return xmlHashCreate(16);
}

/* stolen from "Secure Programming Cookbook for C and C++" */
#define BASE16_TO_10(x) (((x) >= '0' && (x) <= '9') ? ((x) - '0') : (toupper((x)) - 'A' + 10))
   
static xmlChar *decodeUrl(const xmlChar *url, size_t *nbytes) 
{
	xmlChar *out, *ptr;
	const xmlChar *c;
   
	if (!(out = ptr = (xmlChar*) XPL_MALLOC(xmlStrlen(url)+1)))
		return NULL;
	for (c = url;  *c;  c++) 
	{
		if (*c != '%' || !isxdigit(c[1]) || !isxdigit(c[2])) 
			*ptr++ = *c;
		else {
			*ptr++ = (BASE16_TO_10(c[1]) * 16) + (BASE16_TO_10(c[2]));
			c += 2;
		}
	}
	*ptr = 0;
	if (nbytes) 
		*nbytes = (ptr - out); /* does not include null byte */
	return out;
}
#undef BASE_16_TO_10

int xplParseParamString(const xmlChar *params, const char *fallbackEncoding, xplParamsPtr ret)
{
	xmlChar *amp_pos; /* a=1_&_b=%E1%E0%EB%EA */
	xmlChar *param; /* b=%E1%E0%EB%EA */
	xmlChar *eq_pos; /* b_=_%E1%E0%EB%EA */
	xmlChar *param_value; /* in browser/page encoding */
	xmlChar *recoded_value; /* in native encoding (utf-8) */

	while (params && *params)
	{
		amp_pos = BAD_CAST xmlStrchr(params, '&');
		if (amp_pos)
			*amp_pos = 0;
		param = BAD_CAST params;
		params = amp_pos? amp_pos+1: NULL;
		if (param && *param)
		{
			/* extract the left part */
			eq_pos = BAD_CAST xmlStrchr(param, '=');
			if (eq_pos && *(eq_pos+1)) /* "normal" param with a value */
			{
				*eq_pos = 0;
				param_value = decodeUrl(eq_pos + 1, NULL);
				if (!xstrIsValidUtf8Sample(param_value, xmlStrlen(param_value), true))
				{
					if (fallbackEncoding)
					{
						recoded_value = NULL;
						xstrIconvString("utf-8", fallbackEncoding, (char*) param_value, (char*) param_value + xmlStrlen(param_value), (char**) &recoded_value, NULL);
						if (recoded_value && !xstrIsValidUtf8Sample(recoded_value, xmlStrlen(recoded_value), true))
						{
							XPL_FREE(recoded_value);
							recoded_value = NULL;
						}
					} else
						recoded_value = NULL;
				} else
					recoded_value = param_value;
				xplParamAddValue(ret, param, recoded_value, XPL_PARAM_TYPE_USERDATA); /* eats recoded_value */
				if (param_value && recoded_value && (param_value != recoded_value))
					XPL_FREE(param_value);
				*eq_pos = '=';
			} 
		}
		if (amp_pos)
			*amp_pos = '&';
	}
	return 1;
}

static void* copyParamsCallback(void *payload, XML_HCBNC xmlChar *name)
{
	UNUSED_PARAM(name);
	return xplParamValuesCopy((xplParamValuesPtr) payload);
}

xplParamsPtr xplParamsCopy(xplParamsPtr params)
{
	return xmlHashCopy((xmlHashTablePtr) params, copyParamsCallback);
}

xplParamValuesPtr xplParamGet(const xplParamsPtr params, const xmlChar* name)
{
	return (xplParamValuesPtr) xmlHashLookup((xmlHashTablePtr) params, name);
}

xmlChar *xplParamGetFirstValue(const xplParamsPtr params, const xmlChar *name)
{
	xplParamValuesPtr values;

	values = xplParamGet(params, name);
	if (!values)
		return NULL;
	if (values->param_nr && xplParamTypeIsAtomic(values->type))
		return (xmlChar*) values->param_tab[0];
	else
		return NULL;
}

static void freeParamsCallback(void* payload, XML_HCBNC xmlChar* name)
{
	UNUSED_PARAM(name);
	xplParamValuesFree((xplParamValuesPtr) payload);
}

xplParamResult xplParamSet(const xplParamsPtr params, const xmlChar* name, const xplParamValuesPtr values)
{
	int ret;
	if (!params)
		return XPL_PARAM_RES_INVALID_INPUT;
	if (!values)
		return XPL_PARAM_RES_INVALID_INPUT;
	ret = xmlHashAddEntry((xmlHashTablePtr) params, name, (void*) values);
	if (ret == -1)
		/* already exists */
		return (!xmlHashUpdateEntry((xmlHashTablePtr) params, name, (void*) values, freeParamsCallback)? XPL_PARAM_RES_OK: XPL_PARAM_RES_INTERNAL_ERROR);
	else
		return XPL_PARAM_RES_OK;
}

static xplParamValuesPtr xplParamLocateCarrier(xplParamsPtr params, const xmlChar *name)
{
	xplParamValuesPtr carrier;
	
	carrier = (xplParamValuesPtr) xmlHashLookup((xmlHashTablePtr) params, name);
	if (!carrier)
	{
		carrier = xplParamValuesCreate();
		if (!carrier)
			return NULL;
		if (xmlHashAddEntry((xmlHashTablePtr) params, name, carrier))
		{
			xplParamValuesFree(carrier);
			return NULL;
		}
	}
	return carrier;
}

xplParamResult xplParamAddValue(xplParamsPtr params, const xmlChar *name, xmlChar *value, xplParamType type)
{
	xplParamValuesPtr carrier;

	if (!params)
		return XPL_PARAM_RES_INVALID_INPUT;
	carrier = xplParamLocateCarrier(params, name);
	if (!carrier)
		return XPL_PARAM_RES_OUT_OF_MEMORY;
	return xplParamValuesAdd(carrier, value, type);
}

xplParamResult xplParamAddFileInfo(xplParamsPtr params, const xmlChar *name, xmlChar *filename, xmlChar *realPath, int size)
{
	xplParamValuesPtr carrier;

	if (!params)
		return XPL_PARAM_RES_INVALID_INPUT;
	carrier = xplParamLocateCarrier(params, name);
	if (!carrier)
		return XPL_PARAM_RES_OUT_OF_MEMORY;
	return xplParamValuesAddFileInfo(carrier, realPath, filename, size);
}

xplParamResult xplParamReplaceValue(xplParamsPtr params, const xmlChar *name, xmlChar *value, xplParamType type)
{
	xplParamValuesPtr carrier;

	if (!params)
		return XPL_PARAM_RES_INVALID_INPUT;
	carrier = (xplParamValuesPtr) xmlHashLookup((xmlHashTablePtr) params, name);
	if (!carrier)
	{
		carrier = xplParamValuesCreate();
		if (!carrier)
			return XPL_PARAM_RES_OUT_OF_MEMORY;
		if (xmlHashAddEntry((xmlHashTablePtr) params, name, carrier))
			return XPL_PARAM_RES_INTERNAL_ERROR;
		return xplParamValuesAdd(carrier, value, type);
	} else {
		return xplParamValuesReplace(carrier, value, type);
	}
}

void xplParamsScan(const xplParamsPtr params, xplParamsScanner f, void *userData)
{
	xmlHashScan((xmlHashTablePtr) params, f, userData);
}

typedef struct _xplParamsToListCtxt 
{
	bool unique;
	xmlNodePtr ret;
	xmlNodePtr tail;
	xplExpectType expect;
	xmlChar const *node_name;
	xmlNsPtr ns;
	xmlNodePtr parent;
	int type_mask;
} xplParamsToListCtxt;

static void xplParamsToListScanner(void *payload, void *data, XML_HCBNC xmlChar *name)
{
	xplParamsToListCtxt *ctxt;
	xplParamValuesPtr values;
	xmlNodePtr cur, vals;
	static const xplQName qname = { NULL, BAD_CAST "Value" };
	
	values = (const xplParamValuesPtr) payload;
	if (!values)
		return;
	ctxt = (xplParamsToListCtxt*) data;
	if (!(values->type & ctxt->type_mask))
		return;
	vals = xplParamValuesToList(values, ctxt->unique, ctxt->expect, qname, ctxt->parent);
	cur = xmlNewDocNode(ctxt->parent->doc, ctxt->ns, ctxt->node_name? ctxt->node_name: name, NULL);
	if (ctxt->node_name)
		xmlNewProp(cur, BAD_CAST "name", name);
	xplSetChildren(cur, vals);
	if (!ctxt->ret)
		ctxt->ret = ctxt->tail = cur;
	else {
		ctxt->tail->next = cur;
		cur->prev = ctxt->tail;
		ctxt->tail = cur;
	}
}

xmlNodePtr xplParamsToList(const xplParamsPtr params, bool unique, xplExpectType expect, const xplQName qname, xmlNodePtr parent, int typeMask)
{
	xplParamsToListCtxt ctxt;

	if (!params)
		return NULL;
	ctxt.expect = expect;
	if (qname.ncname)
	{
		ctxt.ns = qname.ns;
		ctxt.node_name = qname.ncname;
	} else {
		ctxt.node_name = NULL;
		ctxt.ns = NULL;
	}
	ctxt.parent = parent;
	ctxt.ret = ctxt.tail = NULL;
	ctxt.unique = unique;
	ctxt.type_mask = typeMask;
	xplParamsScan(params, xplParamsToListScanner, &ctxt);
	return ctxt.ret;
}

void xplParamsLockValue(xplParamsPtr params, const xmlChar *name, bool doLock)
{
	xplParamValuesPtr carrier;

	if (!params)
		return;
	carrier = xplParamLocateCarrier(params, name);
	carrier->is_locked = doLock;
}

void xplParamsFree(xplParamsPtr params)
{
	xmlHashFree((xmlHashTablePtr) params, freeParamsCallback);
}
