#include <glue.h>
#include <string.h>
#include <libxml/entities.h>
#include <libxml/xmlsave.h>
#include <libxml/tree.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplstring.h>

xmlChar *app_path;

OutputMethod getOutputMethod(xmlChar *output_method)
{
	if (!xmlStrcasecmp(output_method, BAD_CAST OM_XML))
		return OUTPUT_METHOD_XML;
	if (!xmlStrcasecmp(output_method, BAD_CAST OM_HTML))
		return OUTPUT_METHOD_HTML;
	if (!xmlStrcasecmp(output_method, BAD_CAST OM_XHTML))
		return OUTPUT_METHOD_XHTML;
	if (!xmlStrcasecmp(output_method, BAD_CAST OM_TEXT))
		return OUTPUT_METHOD_TEXT;
	if (!xmlStrcasecmp(output_method, BAD_CAST OM_NONE))
		return OUTPUT_METHOD_NONE;
	return getOutputMethod(BAD_CAST DEFAULT_OUTPUT_METHOD);
}

typedef struct _ParseFormCtxt
{
	xplParamsPtr params;
	xmlChar *store_path;
	xmlChar *file_key;
	xmlChar *file_name;
} ParseFormCtxt, *ParseFormCtxtPtr;

static int field_found(const char *key, const char *filename, char *path, size_t pathlen, void *user_data)
{
	ParseFormCtxtPtr parse_ctxt = (ParseFormCtxtPtr) user_data;

	if (filename && *filename)
	{
		printf("about to create file %s...\n", filename);
		parse_ctxt->file_key = BAD_CAST XPL_STRDUP(key);
		parse_ctxt->file_name = BAD_CAST XPL_STRDUP(filename);
		return MG_FORM_FIELD_STORAGE_STORE; /* file */
	} else
		return MG_FORM_FIELD_STORAGE_GET; /* regular parameter */
}

static int field_get(const char *key, const char *value, size_t valuelen, void *user_data)
{
	ParseFormCtxtPtr parse_ctxt = (ParseFormCtxtPtr) user_data;

	xplParamAddValue(parse_ctxt->params, BAD_CAST key, BAD_CAST XPL_STRDUP(value), XPL_PARAM_TYPE_USERDATA);
	return MG_FORM_FIELD_HANDLE_NEXT; /* TODO how do we determine there're more chunks? */
}

static int field_store(const char *path, long long file_size, void *user_data)
{
	ParseFormCtxtPtr parse_ctxt = (ParseFormCtxtPtr) user_data;

	if (parse_ctxt->file_key)
		xplParamAddFileInfo(parse_ctxt->params, parse_ctxt->file_key,
			parse_ctxt->file_name, BAD_CAST XPL_STRDUP(path), file_size);
	else
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	if (parse_ctxt->file_key)
	{
		XPL_FREE(parse_ctxt->file_key);
		parse_ctxt->file_key = NULL;
	}
	if (parse_ctxt->file_name)
		parse_ctxt->file_name = NULL;
	return MG_FORM_FIELD_HANDLE_NEXT;
}

xplParamsPtr buildParams(struct mg_connection *conn, const struct mg_request_info *request_info, xmlChar *session_id)
{
	xplParamsPtr params;
	ParseFormCtxt parse_ctxt;
	struct mg_form_data_handler fdh = {
		.field_found = field_found,
		.field_get = field_get,
		.field_store = field_store,
	};
	int i;

	params = xplParamsCreate();
	/* xplParseParamString() not needed: the query url part is processed by mg_handle_form_request(), too */
	parse_ctxt.params = params;
	parse_ctxt.store_path = NULL; // TODO provide
	fdh.user_data = &parse_ctxt;
	mg_handle_form_request(conn, &fdh); // TODO error handling

	xplParamReplaceValue(params, DOC_ROOT_PARAM, xmlEncodeSpecialChars(NULL, BAD_CAST app_path), XPL_PARAM_TYPE_USERDATA);
	xplParamReplaceValue(params, RESOURCE_PARAM, xmlEncodeSpecialChars(NULL, BAD_CAST request_info->local_uri), XPL_PARAM_TYPE_USERDATA);
	if (!xplParamGet(params, ENCODING_PARAM))
		xplParamAddValue(params, ENCODING_PARAM, BAD_CAST XPL_STRDUP(DEFAULT_OUTPUT_ENC), XPL_PARAM_TYPE_USERDATA);
	if (!xplParamGet(params, OUTPUT_METHOD_PARAM))
		xplParamAddValue(params, OUTPUT_METHOD_PARAM, BAD_CAST XPL_STRDUP(DEFAULT_OUTPUT_METHOD), XPL_PARAM_TYPE_USERDATA);
	xplParamReplaceValue(params, REMOTE_ADDRESS_PARAM, BAD_CAST XPL_STRDUP(request_info->remote_addr), XPL_PARAM_TYPE_USERDATA);

	xplParamsLockValue(params, DOC_ROOT_PARAM, TRUE);
	xplParamsLockValue(params, REMOTE_ADDRESS_PARAM, TRUE);
	xplParamsLockValue(params, RESOURCE_PARAM, TRUE);

	for (i = 0; i < request_info->num_headers; i++)
		xplParamAddValue(params, BAD_CAST request_info->http_headers[i].name, BAD_CAST XPL_STRDUP(request_info->http_headers[i].value), XPL_PARAM_TYPE_HEADER);

	return params;
}

/* начинка libxml2 */
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

void serializeDoc(struct mg_connection *conn, xmlDocPtr doc, xmlChar *encoding, OutputMethod om)
{
#define ENCODING_ERROR_NODE_STR "<Error>Cannot save document using the specified encoding \"%s\"</Error>"
#define ENCODING_ERROR_STR "Cannot save document using the specified encoding \"%s\""
	xmlBufferPtr buf;
	int save_opts = 0;
	xmlSaveCtxtPtr save_ctxt;
	size_t iconv_size;
	xmlChar *txt;
	char* out_txt = NULL;
	xmlNodePtr doc_root;

	/* prevent caching of a dynamic document */
	mg_printf(conn, "Cache-control: no-cache\r\n");
	switch (om)
	{
		case OUTPUT_METHOD_XML:
		case OUTPUT_METHOD_HTML:
		case OUTPUT_METHOD_XHTML:
			switch (om)
			{
				case OUTPUT_METHOD_XML: save_opts = XML_SAVE_AS_XML; break;
				case OUTPUT_METHOD_HTML: save_opts = XML_SAVE_AS_HTML; break;
				case OUTPUT_METHOD_XHTML: save_opts = XML_SAVE_XHTML; break;
				default: break;
			}
			buf = xmlBufferCreate();
			save_ctxt = xmlSaveToBuffer(buf, (const char*) encoding, save_opts);
			if (save_ctxt)
			{
				xmlSaveSetEscape(save_ctxt, NULL);
				xmlSaveSetAttrEscape(save_ctxt, NULL); /* осторожнее со включением, опасно. */
				xmlSaveDoc(save_ctxt, doc);
				/* xmlSaveClose не освобождает ресурсы, связанные с перекодировкой.
				Видимо, для того, чтобы прикладной программист получил удовольствие
				от копания в исходниках и выноса кишок контекста наружу.
				*/
				if (save_ctxt->handler)
					xmlCharEncCloseFunc(save_ctxt->handler);
				xmlSaveClose(save_ctxt);
				mg_printf(conn, "Content-Length: %d\r\n\r\n", buf->use);
				mg_write(conn, buf->content, xmlStrlen(buf->content));
			} else {
				/*mg_printf(conn, "Content-Length: %d\r\n\r\n", strlen(ERROR_STR) - 2 + xmlStrlen(encoding));*/
				mg_printf(conn, ENCODING_ERROR_NODE_STR, encoding);
			}
			xmlBufferFree(buf);
			break;
		case OUTPUT_METHOD_TEXT:
			doc_root = doc->children;
			while (doc_root)
			{
				if (doc_root->type == XML_ELEMENT_NODE)
					break;
				doc_root = doc_root->next;
			}
			if (doc_root)
				txt = xmlNodeListGetString(doc, doc_root->children, 1);
			else
				txt = NULL;
			if (txt)
			{
				if (xstrIconvString((char*) encoding, "utf-8", (char*) txt, (char*) txt + xmlStrlen(txt), &out_txt, &iconv_size) == -1)
				{
					mg_printf(conn, "Content-Length: %ld\r\n\r\n", strlen(ENCODING_ERROR_STR) - 2 + xmlStrlen(encoding));
					mg_printf(conn, ENCODING_ERROR_STR, encoding);
				} else {
					mg_printf(conn, "Content-Length: %ld\r\n\r\n", iconv_size);
					mg_write(conn, out_txt, iconv_size);
					if (BAD_CAST out_txt != txt)
						XPL_FREE(out_txt);
				}
				XPL_FREE(txt);
			} else
				mg_printf(conn, "Content-Length: 0\r\n\r\n");
			break;
		case OUTPUT_METHOD_NONE:
			mg_printf(conn, "Content-Length: 0\r\n\r\n");
			break;
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
	}
#undef ENCODING_ERROR_NODE_STR
#undef ENCODING_ERROR_STR
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
				mg_printf(conn, "Fri, 10-Jun-1983 03:56:00 GMT;");
			else {
				cur_time = time(NULL);
				cur_time += cfgSessionLifetime;
				cur_time = mktime(gmtime(&cur_time)); // TODO review
				(void) strftime(expires, sizeof(expires), "%a, %d %b %Y %H:%M:%S GMT", localtime(&cur_time));
				mg_printf(conn, "%s", expires);
			}
			mg_printf(conn, "\r\n");
		}
		xplMarkSessionAsSeen(session);
	}
}

int serveXpl(struct mg_connection *conn, void *user_data)
{
/*
	mg_printf(conn, "%s", "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
	printf("uri %s, query %s, remote user %s\n", request_info->uri, request_info->query_string, request_info->remote_user);
	int i;
	for (i = 0; i < request_info->num_headers; i++)
		printf(" * %s = %s\n", request_info->http_headers[i].name, request_info->http_headers[i].value);
	return;
*/
	LEAK_DETECTION_PREPARE
	/* ToDo: исправить работу с кодировками */
	const struct mg_request_info *request_info;
	xplDocumentPtr doc = NULL;
	xmlChar *uri;
	const char *cookies;
	char session_id[XPL_SESSION_ID_SIZE+1];
	xplSessionPtr session = NULL;
	xplParamsPtr params = NULL;
	xmlChar *encoding = NULL;
	xmlChar *output_method = NULL;
	xmlChar *content_type = NULL;
	OutputMethod om;
	xplError ret;
	int http_code;

	LEAK_DETECTION_START();
	request_info = mg_get_request_info(conn);
	/* session */
	cookies = mg_get_header(conn, "Cookie");
	if (cookies && mg_get_cookie(cookies, SESSION_ID_COOKIE, session_id, sizeof(session_id)) > 0)
			session = xplSessionCreate(BAD_CAST session_id);
	if (!session)
		session = xplSessionCreateWithAutoId();
	/* params */
	params = buildParams(conn, request_info, xplSessionGetId(session));
	/* ToDo encoding */
	if (app_path && *app_path && strstr(request_info->local_uri, (char*) app_path) == request_info->local_uri)
		uri = BAD_CAST request_info->local_uri + xmlStrlen(app_path);
	else
		uri = BAD_CAST request_info->local_uri;

	ret = xplProcessFileEx(app_path, uri, params, session, &doc);

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
	if (!output_method || !*output_method)
		output_method = BAD_CAST DEFAULT_OUTPUT_METHOD;
	om = getOutputMethod(output_method);
	if (!content_type || !*content_type)
	{
		switch(om)
		{
			case OUTPUT_METHOD_XML: content_type = BAD_CAST "text/xml"; break;
			case OUTPUT_METHOD_HTML: content_type = BAD_CAST "text/html"; break;
			case OUTPUT_METHOD_XHTML: content_type = BAD_CAST "application/xhtml+xml"; break;
			case OUTPUT_METHOD_TEXT: content_type = BAD_CAST "text/plain"; break;
			case OUTPUT_METHOD_NONE: break;
			default: DISPLAY_INTERNAL_ERROR_MESSAGE();
		}
	}
	switch(ret)
	{
		case XPL_ERR_FATAL_CALLED:
		case XPL_ERR_NO_ERROR:
			if (doc->response)
				mg_printf(conn, "%s\n", doc->response);
			else
				mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: %s; charset=%s\r\n", content_type, encoding);
			setSessionCookie(conn, doc->session);
			if (!doc->response)
				serializeDoc(conn, doc->document, encoding, om);
			else
				mg_printf(conn, "\r\n");
			http_code = 200;
			break;
		case XPL_ERR_INVALID_DOCUMENT:
			/* always return error as XML */
			mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/xml; charset=%s\r\n", encoding);
			if (doc && doc->document)
				serializeDoc(conn, doc->document, encoding, om);
			http_code = 500;
			break;
		default:
			mg_printf(conn, "%s", "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\n");
			mg_printf(conn, "Unknown XPL processor return code: %d", ret);
			http_code = 500;
	}
done:
	if (doc && doc->prologue)
		xplDocumentFree(doc->prologue);
	if (doc && doc->epilogue)
		xplDocumentFree(doc->epilogue);
	if (doc)
		xplDocumentFree(doc);
	if (params)
		xplParamsFree(params);
	xmlResetLastError();
	LEAK_DETECTION_STOP_AND_REPORT();

	return http_code;
}
