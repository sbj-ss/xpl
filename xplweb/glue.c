#include <glue.h>
#include <string.h>
#include <libxml/entities.h>
#include <libxml/xmlsave.h>
#include <libxml/tree.h>
#include <libxpl/abstraction/xef.h>
#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplstring.h>

xmlChar *doc_root;

#define DEFAULT_OUTPUT_METHOD_NAME (BAD_CAST "xhtml")

static OutputMethodDesc output_methods[] =
{
	{ // the first item is the default one
		.name = DEFAULT_OUTPUT_METHOD_NAME,
		.content_type = BAD_CAST "application/xhtml+xml",
		.xml_format = XML_SAVE_XHTML,
		.serializer = OS_XML
	}, {
		.name = BAD_CAST "xml",
		.content_type = BAD_CAST "text/xml",
		.xml_format = XML_SAVE_AS_XML,
		.serializer = OS_XML
	}, {
		.name = BAD_CAST "html",
		.content_type = BAD_CAST "text/html",
		.xml_format = XML_SAVE_AS_HTML,
		.serializer = OS_XML
	}, {
		.name = BAD_CAST "text",
		.content_type = BAD_CAST "text/plain",
		.xml_format = 0,
		.serializer = OS_TEXT
	}, {
		.name = BAD_CAST "none",
		.content_type = BAD_CAST "",
		.xml_format = 0,
		.serializer = OS_NONE
	}
};


OutputMethodDescPtr getOutputMethod(xmlChar *name)
{
	int i;

	if (!name || !*name)
		return &output_methods[0];
	for (i = 0; i < sizeof(output_methods) / sizeof(output_methods[0]); i++)
		if (!xmlStrcasecmp(name, output_methods[i].name))
			return &output_methods[i];
	return NULL;
}

static void _provideUniqueFilename(char* path)
{
#define FN_LEN 16
// TODO make this configurable
#define UPLOAD_FOLDER "upload"
	xefCryptoRandomParams rp;
	xmlChar name_bytes[FN_LEN];
	bool flag = true;
	size_t root_len;

	root_len = xmlStrlen(doc_root);
	memcpy(path, doc_root, root_len);
	path[root_len++] = XPR_PATH_DELIM;
	strcpy(&path[root_len], UPLOAD_FOLDER);
	root_len += strlen(UPLOAD_FOLDER);
	path[root_len++] = XPR_PATH_DELIM;
	path[root_len] = 0;
	xprEnsurePathExistence(BAD_CAST path);
	path[root_len + FN_LEN*2] = 0;

	rp.alloc_bytes = false;
	rp.size = FN_LEN;
	rp.bytes = name_bytes;
	rp.secure = false;

	while (flag)
	{
		/* We don't ask for allocation so the only reason for xefCryptoRandom() to fail
		   is lack of entropy. Keep retrying until the function succeeds. */
		while (!xefCryptoRandom(&rp))
		{
			if (rp.error)
				XPL_FREE(rp.error);
			xprSleep(10);
		}
		xstrBufferToHex(rp.bytes, rp.size, false, BAD_CAST &path[root_len]);
		flag = xprCheckFilePresence(BAD_CAST path, false);
	}
#undef UPLOAD_FOLDER
#undef FN_LEN
}

typedef struct _ParseFormCtxt
{
	/* set by caller */
	bool oom;
	xplParamsPtr params;
	/* used internally */
	xmlChar *key;
	xmlBufferPtr value_buf;
	xmlChar *file_name;
} ParseFormCtxt, *ParseFormCtxtPtr;

static bool _fieldFinalize(ParseFormCtxtPtr ctxt)
{
	xplParamResult res;
	xmlChar *value;

	if (!ctxt->key)
		return true;
	value = xmlBufferDetach(ctxt->value_buf);
	res = xplParamAddValue(ctxt->params, ctxt->key, value, XPL_PARAM_TYPE_USERDATA);
	if (res != XPL_PARAM_RES_OK)
		XPL_FREE(value);
	XPL_FREE(ctxt->key);
	ctxt->key = NULL;
	return res == XPL_PARAM_RES_OK;
}

/* called when a field of any type is found */
static int _fieldFound(const char *key, const char *filename, char *path, size_t pathLen, void *userData)
{
	ParseFormCtxtPtr ctxt = (ParseFormCtxtPtr) userData;

	if (ctxt->key && !_fieldFinalize(ctxt)) /* new param */
	{
		ctxt->oom = true;
		return MG_FORM_FIELD_HANDLE_ABORT;
	}
	if (!(ctxt->key = BAD_CAST XPL_STRDUP(key)))
	{
		ctxt->oom = true;
		return MG_FORM_FIELD_STORAGE_ABORT;
	}
	if (filename && *filename)
	{
		_provideUniqueFilename(path);
		printf("about to create file %s -> %s...\n", filename, path); // TODO remove/move to debug level
		if (!(ctxt->file_name = BAD_CAST XPL_STRDUP(filename)))
		{
			ctxt->oom = true;
			return MG_FORM_FIELD_STORAGE_ABORT;
		}
		return MG_FORM_FIELD_STORAGE_STORE; /* file */
	} else
		return MG_FORM_FIELD_STORAGE_GET; /* regular parameter */
}


/* called for every portion of a form field */
static int _fieldGet(const char *key, const char *value, size_t valueLen, void *userData)
{
	ParseFormCtxtPtr ctxt = (ParseFormCtxtPtr) userData;

	xmlBufferAdd(ctxt->value_buf, BAD_CAST value, valueLen);
	return MG_FORM_FIELD_HANDLE_GET;
}

/* called when uploaded file is stored completely */
static int _fieldStore(const char *path, long long fileSize, void *userData)
{
	ParseFormCtxtPtr ctxt = (ParseFormCtxtPtr) userData;

	if (ctxt->key)
		xplParamAddFileInfo(ctxt->params, ctxt->key,
			ctxt->file_name, BAD_CAST XPL_STRDUP(path), fileSize);
	else
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	if (ctxt->key)
	{
		XPL_FREE(ctxt->key);
		ctxt->key = NULL;
	}
	if (ctxt->file_name) /* file_name is eaten by xplParamAddFileInfo() */
		ctxt->file_name = NULL;
	return MG_FORM_FIELD_HANDLE_NEXT;
}

xplParamsPtr buildParams(struct mg_connection *conn, const struct mg_request_info *requestInfo, xmlChar *sessionId)
{
	ParseFormCtxt ctxt;
	struct mg_form_data_handler fdh = {
		.field_found = _fieldFound,
		.field_get = _fieldGet,
		.field_store = _fieldStore,
	};
	int i;

	memset(&ctxt, 0, sizeof(ctxt));
	if (!(ctxt.params = xplParamsCreate()))
		goto error;
	if (!(ctxt.value_buf = xmlBufferCreateSize(2048)))
		goto error;
	xmlBufferSetAllocationScheme(ctxt.value_buf, XML_BUFFER_ALLOC_HYBRID);
	fdh.user_data = &ctxt;
	mg_handle_form_request(conn, &fdh);
	if (!_fieldFinalize(&ctxt))
		goto error;
	if (ctxt.oom)
		goto error;

	if (xplParamReplaceValue(ctxt.params, DOC_ROOT_PARAM, xmlEncodeSpecialChars(NULL, BAD_CAST doc_root), XPL_PARAM_TYPE_USERDATA) != XPL_PARAM_RES_OK)
		goto error;
	if (xplParamReplaceValue(ctxt.params, RESOURCE_PARAM, xmlEncodeSpecialChars(NULL, BAD_CAST requestInfo->local_uri), XPL_PARAM_TYPE_USERDATA) != XPL_PARAM_RES_OK)
		goto error;
	if (!xplParamGet(ctxt.params, ENCODING_PARAM))
		if (xplParamAddValue(ctxt.params, ENCODING_PARAM, BAD_CAST XPL_STRDUP(DEFAULT_OUTPUT_ENC), XPL_PARAM_TYPE_USERDATA) != XPL_PARAM_RES_OK)
			goto error;
	if (!xplParamGet(ctxt.params, OUTPUT_METHOD_PARAM))
		if (xplParamAddValue(ctxt.params, OUTPUT_METHOD_PARAM, BAD_CAST XPL_STRDUP(DEFAULT_OUTPUT_METHOD_NAME), XPL_PARAM_TYPE_USERDATA) != XPL_PARAM_RES_OK)
			goto error;
	if (xplParamReplaceValue(ctxt.params, REMOTE_ADDRESS_PARAM, BAD_CAST XPL_STRDUP(requestInfo->remote_addr), XPL_PARAM_TYPE_USERDATA) != XPL_PARAM_RES_OK)
		goto error;

	xplParamsLockValue(ctxt.params, DOC_ROOT_PARAM, TRUE);
	xplParamsLockValue(ctxt.params, REMOTE_ADDRESS_PARAM, TRUE);
	xplParamsLockValue(ctxt.params, RESOURCE_PARAM, TRUE);

	for (i = 0; i < requestInfo->num_headers; i++)
		if (xplParamAddValue(
			ctxt.params,
			BAD_CAST requestInfo->http_headers[i].name,
			BAD_CAST XPL_STRDUP(requestInfo->http_headers[i].value),
			XPL_PARAM_TYPE_HEADER
		) != XPL_PARAM_RES_OK)
			goto error;
	goto done;
error:
	if (ctxt.params)
	{
		xplParamsFree(ctxt.params);
		ctxt.params = NULL;
	}
	if (ctxt.key)
		XPL_FREE(ctxt.key);
	if (ctxt.file_name)
		XPL_FREE(ctxt.file_name);
done:
	if (ctxt.value_buf)
		xmlBufferFree(ctxt.value_buf);
	return ctxt.params;
}

/* libxml2 guts */
struct _xmlSaveCtxt {
    void *_private;
    int type;
    int fd;
    const xmlChar *filename;
    const xmlChar *encoding;
    xmlCharEncodingHandlerPtr handler;
    xmlOutputBufferPtr buf;
    xmlDocPtr doc;
    int options;
    int level;
    int format;
    char indent[60 + 1];	/* array for indenting output */
    int indent_nr;
    int indent_size;
    xmlCharEncodingOutputFunc escape;	/* used for element content */
    xmlCharEncodingOutputFunc escapeAttr;/* used for attribute content */
};
typedef struct _xmlSaveCtxt *xmlSaveCtxtPtr;

xmlChar* serializeDoc(xmlDocPtr doc, xmlChar *encoding, OutputMethodDescPtr om, size_t *size)
{
	xmlBufferPtr buf;
	xmlSaveCtxtPtr save_ctxt;
	xmlNodePtr doc_root;
	xmlChar *txt, *ret = NULL;

	if (om->serializer == OS_XML)
	{
		buf = xmlBufferCreate();
		save_ctxt = xmlSaveToBuffer(buf, (const char*) encoding, om->xml_format);
		if (save_ctxt)
		{
			xmlSaveSetEscape(save_ctxt, NULL);
			xmlSaveSetAttrEscape(save_ctxt, NULL);
			xmlSaveDoc(save_ctxt, doc);
			/* xmlSaveClose leaks this */
			if (save_ctxt->handler)
				xmlCharEncCloseFunc(save_ctxt->handler);
			xmlSaveClose(save_ctxt);
			*size = xmlBufferLength(buf);
			ret = xmlBufferDetach(buf);
		} else {
			ret = xplFormatMessage(BAD_CAST "<Error>Cannot save document using the specified encoding \"%s\"</Error>", encoding);
			*size = xmlStrlen(ret);
		}
		xmlBufferFree(buf);
	} else if (om->serializer == OS_TEXT) {
		doc_root = xplFirstElementNode(doc->children);
		txt = doc_root? xmlNodeListGetString(doc, doc_root->children, 1): NULL;
		if (txt)
		{
			if (!xmlStrcmp(encoding, BAD_CAST "utf-8"))
			{
				ret = txt;
				*size = xmlStrlen(txt);
			} else if (xstrIconvString((char*) encoding, "utf-8", (char*) txt, (char*) txt + xmlStrlen(txt), (char**) &ret, size) == -1) {
				ret = xplFormatMessage(BAD_CAST "Cannot save document using the specified encoding \"%s\"", encoding);
				*size = xmlStrlen(ret);
			} else /* txt and size set in xstrIconvString() */
				XPL_FREE(txt);
		} else {
			ret = NULL;
			*size = 0;
		}
	} else {
		ret = NULL;
		*size = 0;
	}
	return ret;
}

void setSessionCookie(struct mg_connection *conn, xplSessionPtr session)
{
	time_t cur_time;
	char expires[64];

	if (session)
	{
		if (xplSessionIsJustCreated(session) || !xplSessionIsValid(session))
		{
			mg_printf(conn, "Set-Cookie: session_id=%s; path=/; expires=", xplSessionGetId(session));
			if (!xplSessionIsValid(session))
				mg_printf(conn, "Fri, 10-Jun-1983 10:56:00 GMT;");
			else {
				cur_time = time(NULL);
				cur_time += cfgSessionLifetime;
				cur_time = mktime(gmtime(&cur_time)); // TODO review
				(void) strftime(expires, sizeof(expires), "%a, %d %b %Y %H:%M:%S GMT", localtime(&cur_time));
				mg_printf(conn, "%s", expires);
			}
			mg_printf(conn, "\r\n");
		}
		xplSessionMarkAsSeen(session);
	}
}

int serveXpl(struct mg_connection *conn, void *userData)
{
	LEAK_DETECTION_PREPARE
	const struct mg_request_info *request_info;
	xplDocumentPtr doc = NULL;
	xmlChar *uri;
	const char *cookies;
	char session_id[XPL_SESSION_ID_SIZE*2 + 1];
	xplSessionPtr session = NULL;
	xplParamsPtr params;
	xmlChar *encoding = NULL;
	xmlChar *output_method = NULL;
	xmlChar *content_type = NULL;
	xmlChar *payload = NULL;
	size_t payload_size;
	OutputMethodDescPtr om;
	xplError ret;
	int http_code;

	LEAK_DETECTION_START();
	request_info = mg_get_request_info(conn);
	cookies = mg_get_header(conn, "Cookie");
	if (cookies && mg_get_cookie(cookies, SESSION_ID_COOKIE, session_id, sizeof(session_id)) > 0)
		if (!(session = xplSessionCreateOrGetShared(BAD_CAST session_id)))
		{
			mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\n\r\n");
			goto done;
		}
	if (!(params = buildParams(conn, request_info, xplSessionGetId(session))))
	{
		mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\n\r\n");
		goto done;
	}
	/* ToDo encoding */
	if (doc_root && *doc_root && strstr(request_info->local_uri, (char*) doc_root) == request_info->local_uri)
		uri = BAD_CAST request_info->local_uri + xmlStrlen(doc_root);
	else
		uri = BAD_CAST request_info->local_uri;

	ret = xplProcessFileEx(doc_root, uri, params, session, &doc);

	if (!doc)
	{
		mg_printf(conn, "%s", "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\n");
		mg_printf(conn, "Internal error: couldn't allocate service structures to parse %s", request_info->local_uri);
		http_code = 500;
		goto done;
	}
	/* even if processing failed, we should respect user preferences */
	if (params)
	{
		encoding = xplParamGetFirstValue(params, ENCODING_PARAM);
		content_type = xplParamGetFirstValue(params, CONTENT_TYPE_PARAM);
		output_method = xplParamGetFirstValue(params, OUTPUT_METHOD_PARAM);
	}
	if (!encoding || !*encoding)
		encoding = BAD_CAST DEFAULT_OUTPUT_ENC;
	om = getOutputMethod(output_method);
	if (!content_type || !*content_type)
		content_type = om->content_type;
	if (ret == XPL_ERR_NO_ERROR || ret == XPL_ERR_FATAL_CALLED)
	{
		if (doc->response)
			mg_printf(conn, "%s\r\n\r\n", doc->response);
		else {
			if ((payload = serializeDoc(doc->document, encoding, om, &payload_size)))
			{
				http_code = 200;
				mg_printf(conn, "HTTP/1.1 200 OK\r\n");
				mg_printf(conn, "Cache-control: no-cache\r\n");
				mg_printf(conn, "Content-Length: %lu\r\n", payload_size);
				mg_printf(conn, "Content-Type: %s; charset=%s\r\n", content_type, encoding);
				setSessionCookie(conn, doc->shared_session);
				mg_printf(conn, "\r\n");
				mg_printf(conn, "%s", payload);
				XPL_FREE(payload);
			}
		}
		http_code = 200;
	} else {
		/* always return error as XML */
		mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\n");
		mg_printf(conn, "Content-Type: text/xml; charset=%s\r\n", encoding);
		if (doc && doc->document && (payload = serializeDoc(doc->document, encoding, om, &payload_size)))
		{
			mg_printf(conn, "Cache-control: no-cache\r\n");
			mg_printf(conn, "Content-Length: %lu\r\n", payload_size);
			mg_printf(conn, "\r\n");
			mg_printf(conn, "%s", payload);
			XPL_FREE(payload);
		} else
			mg_printf(conn, "\r\n");
		http_code = 500;
	}
done:
	if (doc)
		xplDocumentFree(doc);
	if (params)
		xplParamsFree(params);
	xmlResetLastError();
	LEAK_DETECTION_STOP_AND_REPORT();

	return http_code;
}
