#include "stdafx.h"
#include "Configuration.h"
#include <stdlib.h>
#include <fcntl.h>
#include <Share.h>
#include <io.h>
#include <sys/stat.h>
#include "abstraction/ExtFeatures.h"
#ifdef _USE_TCMALLOC
# include "tcmalloc.h"
#endif
#ifdef _WIN32
# include <Winsock2.h>
#endif

#pragma warning(disable: 4244)

#define OM_XML "xml"
#define OM_HTML "html"
#define OM_XHTML "xhtml"
#define OM_TEXT "text"
#define OM_NONE "none"

#define DEFAULT_OUTPUT_ENC "windows-1251"
/* Хорошо подумайте, прежде чем менять этот параметр. */
#define DEFAULT_OUTPUT_METHOD OM_HTML
#define CONFIG_FILE "xplweb.conf"

#define ENCODING_PARAM (BAD_CAST "Encoding")
#define CONTENT_TYPE_PARAM (BAD_CAST "ContentType")
#define OUTPUT_METHOD_PARAM (BAD_CAST "OutputMethod")

typedef enum { 
	OUTPUT_METHOD_XML,
	OUTPUT_METHOD_HTML,
	OUTPUT_METHOD_XHTML,
	OUTPUT_METHOD_TEXT,
	OUTPUT_METHOD_NONE
} OutputMethod;

/* Мерзкие синглтоны :) */
static int exit_flag;
struct mg_context *ctx;
xmlChar *app_path;
XPR_EVENT_HANDLE lock_threads_event;
xprSmartSemaphore worker_threads_semaphore;
XPR_FS_CHAR *pid_file_name = NULL;

void shutdownServer(void);

static void signal_handler(int sig_num)
{	
	exit_flag = sig_num;
}

#ifdef _WIN32
static BOOL WINAPI win_signal_handler(DWORD reason)
{
	exit_flag = reason;
	shutdownServer();
	return TRUE;
}
#endif

void lockThreads(BOOL doLock)
{
	if (doLock)
		XPR_RESET_EVENT(lock_threads_event);
	else
		XPR_SET_EVENT(lock_threads_event);
	xprWaitSmartSemaphore(&worker_threads_semaphore);
}

static xmlChar* findEncoding(const mg_request_info *request_info)
{
	int i;
	char *langs;
	for (i = 0; i < request_info->num_headers; i++)
	{
		if (!strcmp(request_info->http_headers[i].name, "Accept-Charset"))
		{
			langs = request_info->http_headers[i].value;
			char *comma_pos = strchr(langs, ',');
			if (comma_pos)
				return xmlStrndup(BAD_CAST langs, comma_pos - langs);
			else
				return NULL;
		}
	}
	return NULL;
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

static OutputMethod getOutputMethod(xmlChar *output_method)
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

static void serializeDoc(struct mg_connection *conn, xmlDocPtr doc, xmlChar *encoding, OutputMethod om)
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
				if (iconv_string((char*) encoding, "utf-8", (char*) txt, (char*) txt + xmlStrlen(txt), &out_txt, &iconv_size) == -1) 
				{
					mg_printf(conn, "Content-Length: %d\r\n\r\n", strlen(ENCODING_ERROR_STR) - 2 + xmlStrlen(encoding));
					mg_printf(conn, ENCODING_ERROR_STR, encoding);
				} else {
					mg_printf(conn, "Content-Length: %d\r\n\r\n", iconv_size);
					mg_write(conn, out_txt, iconv_size);
					if (BAD_CAST out_txt != txt)
						xmlFree(out_txt);
				}
				xmlFree(txt);
			} else 
				mg_printf(conn, "Content-Length: 0\r\n\r\n");
			break;
		case OUTPUT_METHOD_NONE:
			mg_printf(conn, "Content-Length: 0\r\n\r\n");
			break;
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE
	}
#undef ENCODING_ERROR_STR
}

static xmlChar *makePath(const xmlChar* fn)
{
	xmlChar *slash_pos = BAD_CAST strrchr((const char*) fn + 1, XPR_PATH_DELIM);
	if (!slash_pos)
		return xmlStrdup(app_path);
	int path_len = xmlStrlen(app_path) + (slash_pos - fn);
	xmlChar *ret = (xmlChar*) xmlMalloc(path_len);
	strcpy((char*) ret, (const char*) app_path);
	strncat((char*) ret, (const char*) fn + 1, slash_pos - fn - 1);
	return ret;
}

static xmlChar *justName(const xmlChar *fn)
{
	return BAD_CAST strrchr((const char*) fn, XPR_PATH_DELIM) + 1;
}

/* Разбор multipart-форм. АдЪ и Израиль. */
inline void safeIncrement(char** c, int count)
{
	while (count-- && *(*c++));
}

static char* extractBoundary(const char* ct)
{
	const char *value_start;
	int ret_len;
	char *ret;

	value_start = strstr(ct, "boundary=");
	if (!value_start)
		return NULL;
	value_start += 9; /* Здесь не нужен safeIncrement, раз уж строку нашло */
	ret_len = strlen(value_start);
	ret = (char*) xmlMalloc(ret_len + 3);
	if (!ret)
		return NULL;
	*ret++ = '-';
	*ret++ = '-';
	strcpy(ret, value_start);
	return ret - 2;
}

typedef struct _mpfParseParams
{
	char *start;			/* Начало блока разбора */
	char *end;				/* Конец блока разбора */
	char *boundary;			/* Разделитель форм (с префиксом "--"), освобождается в parseMultipartForm */
	char *name;				/* Имя внешнего параметра (для вложенных аттачей) */
	int boundary_len;		/* Предвычисленная длина разделителя */
	xmlChar *save_path;		/* Путь для сохранения файловых параметров */
	xplParamsPtr params;	/* Структура URL-параметров */
} mpfParseParams, *mpfParseParamsPtr;

/* Поиск нуль-терминированной подстроки в указанном диапазоне.
   В указанном диапазоне допустимы нули.
 */
static char* safeStrstr(char *start, char *end, char *what)
{
	char *c, *cw;

	if (!what)
		return NULL;
	c = start;
	cw = what;
	while (c <= end)
	{
		if (*c == *cw)
			cw++;
		else
			cw = what;
		if (!*cw)
			return c - (cw - what) + 1;
		c++;
	}
	return NULL;
}

/* Разбор параметра формы. Считаем, что start сразу указывает на него. */
static void parseUserdataParam(mpfParseParamsPtr params)
{
	xmlChar *value;

	value = (xmlChar*) xmlMalloc(params->end - params->start + 2);
	*value = 0;
	xmlStrncat(value, BAD_CAST params->start, params->end - params->start + 1);
	xplParamAddValue(params->params, (const xmlChar*) params->name, value, XPL_PARAM_TYPE_USERDATA);
}

/* Разбор нефайлового параметра. Считаем, что он начинается или с перевода строки
   (параметр формы), или с Content-Type (вложенная multipart-форма)
 */
static void parseNonfileParam(mpfParseParamsPtr params)
{
/*	char *c1, *c2, *prev_boundary;*/

	if (params->end - params->start < 2)
		return;
	if ((*params->start == 0x0D) && (*(params->start+1) == 0x0A))
	{
		params->start += 2;
		parseUserdataParam(params); /* Нет типа, параметр формы */
	} else { /* ToDo: Проверим тип, может быть вложенность */

	}
}

#define UFN_MAX_BUF_SIZE 4096
xmlChar *unique_fn(const xmlChar *baseDir, const xmlChar *oldName, size_t *fs_buf_len)
{
	xmlChar test_buf[UFN_MAX_BUF_SIZE], *test_buf_ptr, *fs_buf;
	const char *ext;
	unsigned int hex;
	int file_handle;
	BOOL flag = TRUE;
	size_t test_buf_len;

	ext = strrchr((char*) oldName, '.');
	test_buf_len = xmlStrlen(baseDir) + strlen(ext) + 10;
	if (test_buf_len > UFN_MAX_BUF_SIZE)
		return NULL;
	strcpy_s((char*) test_buf, UFN_MAX_BUF_SIZE, (char*) baseDir);
	test_buf_ptr = &test_buf[0];
	test_buf_ptr += xmlStrlen(baseDir);
	*test_buf_ptr++ = XPR_PATH_DELIM;
	if (ext)
		strcpy((char*) &test_buf_ptr[8], ext);
	while (flag)
	{
		rand_s(&hex);
		_snprintf((char*) test_buf_ptr, 8, "%08X", hex);
		fs_buf = NULL;
		hex = iconv_string(XPR_FS_ENCODING, "utf-8", (const char*) test_buf, (const char*) test_buf + test_buf_len, (char**) &fs_buf, fs_buf_len);
		XPR_FILE_SOPEN(&file_handle, (const XPR_FS_CHAR*) fs_buf, _O_RDONLY, _SH_DENYNO, 0);
		if (file_handle == -1)
			flag = FALSE;
		else {
			XPR_FILE_CLOSE(file_handle);
			xmlFree(fs_buf);
		}
	}
	return fs_buf;
}

/* Разбор файлового параметра. Считаем, что start указывает на filename. */
static void parseFileParam(mpfParseParamsPtr params)
{
	char *c1, *c2;
	xmlChar *filename = NULL, *real_path, *utf8path, *console_path;
	int size;
	size_t real_path_len;
	int fd;

	if (!(c1 = safeStrstr(params->start, params->end, "filename=\"")))
		return; /* Кривой заголовок */
	c1 += 10;
	if (!(c2 = safeStrstr(c1, params->end, "\"")))
		return; /* Кривой заголовок */
	if (c1 == c2)
		return; /* Пустое поле ввода */
	/* ToDo: may be utf-8! */
	if (iconv_string("utf-8", DEFAULT_OUTPUT_ENC, c1, c2, (char**) &filename, NULL) == -1)
		return; /* Кто-то напутал с кодировками */
	/* ToDo: проверить Content-Type, может быть представление в base64 etc */
	c2 += 3; /* Кавычка, перевод строки */
	if (c2 > params->end)
		return;
	if (!(c2 = safeStrstr(c2, params->end, "\r\n")))
		return;
	c2 += 4; /* Два перевода строки */
	if (c2 > params->end)
		return;
	size = params->end - c2 + 1;

	real_path = unique_fn(params->save_path, filename, &real_path_len);
	if (!real_path)
		return;
	fd = _wsopen((const wchar_t*) real_path, _O_CREAT | _O_RDWR | _O_BINARY, _SH_DENYWR, _S_IREAD | _S_IWRITE);
	if (fd == -1)
	{
		console_path = NULL;
		if (iconv_string(XPR_CONSOLE_ENCODING, XPR_FS_ENCODING, (const char*) real_path, (const char*) real_path + real_path_len, (char**) &console_path, NULL) == -1)
			xplDisplayMessage(xplMsgWarning, BAD_CAST "cannot create uploaded file (and cannot show its name using console encoding)");
		else {
			xplDisplayMessage(xplMsgWarning, BAD_CAST "cannot create uploaded file \"%s\"", console_path);
			xmlFree(console_path);
		}
		xmlFree(real_path);
		return;
	}
	_write(fd, c2, size);
	_close(fd);
	utf8path = NULL;
	iconv_string("utf-8", XPR_FS_ENCODING, (char*) real_path, (char*) real_path + real_path_len, (char**) &utf8path, NULL);
	xplParamAddFileInfo(params->params, BAD_CAST params->name, filename, utf8path, size);
	xmlFree(real_path);
}

#define CD_FORM_DATA 1
#define CD_ATTACHMENT 2

/* Разбор блока формы. Считаем, что он начинается с Content-Disposition. */
static void parseFormPart(mpfParseParamsPtr params)
{
	char *c1, *c2, *prev_name = NULL;
	int cd;

	if (params->end <= params->start)
		return;
	if (!(c1 = safeStrstr(params->start, params->end, "Content-Disposition: ")))
		return;
	c1 += 21;
	if (c1 > params->end)
		return;
	if (!(c2 = safeStrstr(c1, params->end, ";")))
		return;
	/* Здесь безопасно использовать strncmp, т.к. точка с запятой в пределах блока */
	if (!strncmp(c1, "form-data", 9))
		cd = CD_FORM_DATA;
	else if (!strncmp(c1, "attachment", 10))
		cd = CD_ATTACHMENT;
	else
		return; /* Фигня якась */
	/* Здесь мы можем портить входной блок параметров */
	c2 += 2; /* точка с запятой, пробел */
	if (cd == CD_FORM_DATA) /* Имя предусмотрено */
	{
		if (!safeStrstr(c2, params->end, "name=\""))
			return; /* Кривой заголовок */
		c2 += 6;
		if (c2 >= params->end)
			return; /* Кривой заголовок */
		if (!(c1 = safeStrstr(c2, params->end, "\"")))
			return; /* Кривой заголовок */
		prev_name = params->name; /* В блоке могут сочетаться параметры и набор аттачей */
		params->name = (char*) xmlMalloc(c1 - c2 + 1);
		strncpy(params->name, c2, c1 - c2);
		*(params->name + (c1 - c2)) = 0;
		params->start = c1 + 3; /* Кавычка, точка с запятой+пробел или перевод строки */
	} else
		params->start = c2; /* аттач, нет имени (оно уровнем выше) */
	if (cd == CD_ATTACHMENT) /* Однозначно файл */
	{
		parseFileParam(params);
	} else if (cd == CD_FORM_DATA) { /* Нужна доп. проверка */
		if (safeStrstr(params->start, params->end, "filename=\""))
			parseFileParam(params);
		else
			parseNonfileParam(params);
	}
	if (params->name)
		xmlFree(params->name);
	params->name = prev_name;
}

static void parseMultipartForm(mpfParseParamsPtr params)
{
	mpfParseParams p;
	char *c;

	c = params->start;
	p.boundary = params->boundary;
	p.boundary_len = params->boundary_len;
	p.save_path = params->save_path;
	p.params = params->params;
	p.name = NULL;
	while (c < params->end)
	{
		p.start = safeStrstr(c, params->end, params->boundary); /* Найдём первую границу */
		if (!p.start)
			break; /* Больше нет терминаторов, в данных мусор */
		p.start += params->boundary_len;
		if ((*p.start == '-') && (*(p.start+1) == '-'))
			break; /* Конец формы */
		p.start += 2; /* Перевод строки */
		p.end = safeStrstr(p.start, params->end, params->boundary); /* Найдём следующую границу */
		if (!p.end)
			break; /* Больше нет терминаторов, в данных мусор */
		p.end -= 3; /* Перевод строки и разделитель не входят в блок */
		parseFormPart(&p);
		c = p.end + 3; /* Проход за разобранный блок */
	}
	xmlFree(params->boundary);
}

static void serveXpl(struct mg_connection *conn, const struct mg_request_info *request_info, void *user_data)
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
	xplDocumentPtr doc = NULL;
	/* ToDo: исправить работу с кодировками */
	const char *cookie;
	char session_id[XPL_SESSION_ID_SIZE+1];
	cookie = mg_get_header(conn, "Cookie");
	xplSessionPtr session = NULL;
	xplParamsPtr params = NULL;
	char expires[64];

	/* printf("about to serve\n"); */
	LEAK_DETECTION_START
	XPR_WAIT_EVENT(lock_threads_event, XPR_WAIT_INFINITE);
	xprEnterSmartSemaphore(&worker_threads_semaphore);
	if (cookie && (cookie = strstr(cookie, "session_id=")))
		if (sscanf(cookie, "session_id=%8s", session_id) == 1)
			session = xplSessionCreate(BAD_CAST session_id);
	char *enc = DEFAULT_OUTPUT_ENC;/*findEncoding(request_info);*/
	params = xplParamsCreate();
	xplParseParamString(BAD_CAST request_info->query_string, enc, params);
	const xmlChar* content_type;
	content_type = BAD_CAST mg_get_header(conn, "Content-Type");
	if (xmlStrstr(content_type, BAD_CAST "multipart/form-data"))
	{
		/* В данном случае невыгодно делать нуль-терминированную копию:
		   1. Она может оказаться очень громоздкой
		   2. В случае передачи бинарных файлов с типом octet-stream ничего не будет работать
		 */
		if (!session)
			session = xplSessionCreateWithAutoId(); /* В мультипарте обычно идёт загрузка файлов */
		xmlChar *user_dir_name, *fs_user_dir_name;
		int user_dir_name_len;
		user_dir_name_len = strlen(mg_get_option(ctx, "root")) + XPL_SESSION_ID_SIZE + 16;
		user_dir_name = (xmlChar*) xmlMalloc(user_dir_name_len);
		*user_dir_name = 0;
		strcat((char*) user_dir_name, mg_get_option(ctx, "root"));
		strcat((char*) user_dir_name, XPR_PATH_DELIM_STR);
		strcat((char*) user_dir_name, "_upload");
		strcat((char*) user_dir_name, XPR_PATH_DELIM_STR);
		strcat((char*) user_dir_name, (char*) xplSessionGetId(session));
		fs_user_dir_name = NULL;
		if (iconv_string(XPR_FS_ENCODING, DEFAULT_OUTPUT_ENC, (char*) user_dir_name, (char*) user_dir_name + user_dir_name_len, (char**) &fs_user_dir_name, NULL) != -1)
		{
			if ((_wmkdir((const wchar_t*) fs_user_dir_name) == -1) && (errno != EEXIST))
			{
				xmlChar *console_dir_name = NULL;
				if (iconv_string(XPR_CONSOLE_ENCODING, DEFAULT_OUTPUT_ENC, (char*) user_dir_name, (char*) user_dir_name + user_dir_name_len, (char**) &console_dir_name, NULL) != -1)
				{
					xplDisplayMessage(xplMsgWarning, BAD_CAST "cannot create upload directory \"%s\"", console_dir_name);
					xmlFree(console_dir_name);
				} else
					xplDisplayMessage(xplMsgWarning, BAD_CAST "cannot create upload directory (and can't show its name in console encoding)");
			} /* Если не получилось - не повод всё заваливать */
			xmlFree(fs_user_dir_name);
		} else
			xplDisplayMessage(xplMsgWarning, BAD_CAST "cannot translate upload directory name into filesystem encoding, no directory created");
		mpfParseParams parse_params;
		parse_params.boundary = extractBoundary((char*) content_type);
		parse_params.boundary_len = strlen(parse_params.boundary);
		parse_params.start = request_info->post_data;
		parse_params.end = request_info->post_data + request_info->post_data_len;
		parse_params.params = params;
		parse_params.name = NULL;
		parse_params.save_path = user_dir_name;
		parseMultipartForm(&parse_params);
		xmlFree(user_dir_name);
	} else if (xmlStrstr(content_type, BAD_CAST "application/x-www-form-urlencoded")) {
		/* POST-data aren't NULL-terminated! */
		xmlChar *post_data = xmlStrndup(BAD_CAST request_info->post_data, request_info->post_data_len);
		xplParseParamString(post_data, enc, params);
		xmlFree(post_data);
	}
	/*if (enc) xmlFree(enc);*/
	if (params)
	{
		xplParamReplaceValue(params, BAD_CAST "Resource", xmlEncodeSpecialChars(NULL, BAD_CAST request_info->uri), XPL_PARAM_TYPE_USERDATA);
		xplParamReplaceValue(params, BAD_CAST "DocRoot", xmlEncodeSpecialChars(NULL, BAD_CAST mg_get_option((struct mg_context*) user_data, "root")), XPL_PARAM_TYPE_USERDATA);
		if (!xplParamGet(params, ENCODING_PARAM))
			xplParamAddValue(params, ENCODING_PARAM, xmlStrdup(BAD_CAST DEFAULT_OUTPUT_ENC), XPL_PARAM_TYPE_USERDATA);
		if (!xplParamGet(params, OUTPUT_METHOD_PARAM))
			xplParamAddValue(params, OUTPUT_METHOD_PARAM, xmlStrdup(BAD_CAST DEFAULT_OUTPUT_METHOD), XPL_PARAM_TYPE_USERDATA);
		struct in_addr addr;
		addr.S_un.S_addr = ntohl(request_info->remote_ip);
		xplParamReplaceValue(params, BAD_CAST "RemoteIP", xmlStrdup(BAD_CAST inet_ntoa(addr)), XPL_PARAM_TYPE_USERDATA);
		xplParamsLockValue(params, BAD_CAST "Resource", TRUE);
		xplParamsLockValue(params, BAD_CAST "DocRoot", TRUE);
		xplParamsLockValue(params, BAD_CAST "RemoteIP", TRUE);
		int i;
		for (i = 0; i < request_info->num_headers; i++)
			xplParamAddValue(params, BAD_CAST request_info->http_headers[i].name, xmlStrdup(BAD_CAST request_info->http_headers[i].value), XPL_PARAM_TYPE_HEADER);
	}
	/* xprConvertSlashes(BAD_CAST request_info->uri); */
	/* ToDo encoding */
	xmlChar *uri;
	if (strstr(request_info->uri, (char*) app_path) == request_info->uri)
		uri = BAD_CAST request_info->uri + xmlStrlen(app_path);
	else
		uri = BAD_CAST request_info->uri;
	xplError ret = xplProcessFileEx(app_path, uri, params, session, &doc);
	if (!doc)
	{
		mg_printf(conn, "%s", "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\n");
		mg_printf(conn, "Internal error: couldn't allocate service structures to parse %s", request_info->uri);
		goto done;
	} 
	/* even if processing failed, we should respect user prefs */
	xmlChar *encoding = NULL;
	xmlChar *output_method = NULL;
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
	OutputMethod om = getOutputMethod(output_method);
	if (!content_type || !*content_type)
	{
		switch(om)
		{
			case OUTPUT_METHOD_XML: content_type = BAD_CAST "text/xml"; break;
			case OUTPUT_METHOD_HTML: content_type = BAD_CAST "text/html"; break;
			case OUTPUT_METHOD_XHTML: content_type = BAD_CAST "application/xhtml+xml"/*"text/html"*/; break;
			case OUTPUT_METHOD_TEXT: content_type = BAD_CAST "text/plain"; break;
			/* для OUTPUT_METHOD_NONE ничего делать не нужно */
		}
	}
	switch(ret)
	{
		case XPL_ERR_FATAL_CALLED:
		case XPL_ERR_NO_ERROR:
			if (doc->response)
				mg_printf(conn, "%s", doc->response);
			else
				mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: %s; charset=%s", content_type, encoding);
			session = doc->session;
			if (session)
			{
				if (xplSessionIsJustCreated(session) || !xplSessionIsValid(session))
				{
					mg_printf(conn, "\r\nSet-Cookie: session_id=%s; path=/; expires=", xplSessionGetId(doc->session));
					if (!xplSessionIsValid(session))
						mg_printf(conn, "Fri, 10-Jun-1983 03:56:00 GMT;");
					else {
						time_t cur_time = time(NULL);
						cur_time += cfgSessionLifetime;
						cur_time = mktime(gmtime(&cur_time));
						(void) strftime(expires, sizeof(expires), "%a, %d %b %Y %H:%M:%S GMT", localtime(&cur_time));
						mg_printf(conn, "%s", expires);
					}
				}
				xplMarkSessionAsSeen(session);
			}
			mg_printf(conn, "\r\n");
			if (!doc->response)
				serializeDoc(conn, doc->document, encoding, om);
			else
				mg_printf(conn, "\r\n");
			break;
		case XPL_ERR_INVALID_DOCUMENT:
			/* всегда отдаём ошибку в XML */
			mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/xml; charset=%s\r\n", encoding);
			if (doc && doc->document)
				serializeDoc(conn, doc->document, encoding, om);
			break;
		default:
			mg_printf(conn, "%s", "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\n");
			mg_printf(conn, "Unknown XPL processor return code: %d", ret);
	}
done:
	if (doc && doc->prologue) xplDocumentFree(doc->prologue);
	if (doc && doc->epilogue) xplDocumentFree(doc->epilogue);
	if (doc) xplDocumentFree(doc);
	if (params) xplParamsFree(params);
	xprExitSmartSemaphore(&worker_threads_semaphore);
	LEAK_DETECTION_STOP
}

/* UNIX-style */
void checkPid(void)
{
	int pid_file_name_len;
	char pid_str[12];
	FILE *pid_file;
	int pid;

	if (!pid_file_name)
	{
		/* В libxml2 ещё не инициализировано управление памятью, нельзя использовать xmlMalloc */
		pid_file_name = (XPR_FS_CHAR*) malloc(2048);
#ifdef _WIN32
		if (pid_file_name_len = GetModuleFileNameW(0, pid_file_name, 1024))
		{
			pid_file_name[pid_file_name_len - 3] = XPR_MK_FS_CHAR('p');
			pid_file_name[pid_file_name_len - 2] = XPR_MK_FS_CHAR('i');
			pid_file_name[pid_file_name_len - 1] = XPR_MK_FS_CHAR('d');
		}
#else
#error not implemented
#endif
	}
	if (pid_file = XPR_FOPEN(pid_file_name, XPR_MK_FS_CHAR("r")))
	{
		if (fscanf(pid_file, "%11s", pid_str) != 1)
		{
			xplDisplayMessage(xplMsgWarning, BAD_CAST "Ignoring broken PID file");
		} else {
			pid_str[sizeof(pid_str) - 1] = 0;
			pid = atoi(pid_str);
			if (!pid)
				xplDisplayMessage(xplMsgWarning, BAD_CAST "Ignoring broken PID file");
			else {
				if (xprCheckPid(pid))
				{
					xplDisplayMessage(xplMsgError, BAD_CAST "An instance of server with PID=%d is already running serving the same directory", pid);
					exit(1);
				} else
					xplDisplayMessage(xplMsgInfo, BAD_CAST "Removing stale PID file, PID=%d", pid);
			}
		}
		fclose(pid_file);
	}
	if (!(pid_file = XPR_FOPEN(pid_file_name, XPR_MK_FS_CHAR("w"))))
	{
		xplDisplayMessage(xplMsgWarning, BAD_CAST "Cannot open PID file for writing");
		return;
	}
	fprintf(pid_file, "%d", xprGetPid());
	fclose(pid_file);
}

void removePid(void)
{
	if (pid_file_name)
	{
		(void) XPR_FILE_UNLINK(pid_file_name);
		free(pid_file_name);
	}
}
/* Прикрываем лавочку */
void shutdownServer(void)
{
	/* Что это было?.. */
	/* if (!ctx)
		return;
		*/
	xplDisplayMessage(xplMsgInfo, BAD_CAST "Shutting down XPL interpreter...");
	xplDoneParser();
	xplDisplayMessage(xplMsgInfo, BAD_CAST "Shutting down supplementary layers...");
	XPR_DELETE_EVENT(lock_threads_event);
	xprCleanupSmartSemaphore(&worker_threads_semaphore);
	xefShutdown();
	xprShutdown(XPR_STARTSTOP_EVERYTHING);
	xplDisplayMessage(xplMsgInfo, BAD_CAST "Exiting on signal %d, waiting for all threads to finish...", exit_flag);
	xmlCleanupParser(); /* no messages after the xml parser cleanup! */
	exit_flag = 1;
	removePid();
	mg_stop(ctx);
	fflush(stdout);
	exit(EXIT_SUCCESS);
}

/* from mongoose main.c */
static void show_usage_and_exit(void)
{
	mg_show_usage_string(stderr);
	exit(EXIT_FAILURE);
}

static void process_command_line_arguments(struct mg_context *ctxt, char *argv[])
{
	const char	*config_file = CONFIG_FILE;
	char		line[512], opt[512], val[512], path[FILENAME_MAX], *p;
	FILE		*fp;
	size_t		i, line_no = 0;

	/* First find out, which config file to open */
	for (i = 1; argv[i] != NULL && argv[i][0] == '-'; i += 2)
		if (argv[i + 1] == NULL)
			show_usage_and_exit();

	if (argv[i] != NULL && argv[i + 1] != NULL && argv[i][0] != '+') 
	{
		/* More than one non-option arguments are given */
		show_usage_and_exit();
	} else if (argv[i] != NULL && argv[i][0] != '+') {
		/* Just one non-option argument is given, this is config file */
		config_file = argv[i];
	} else {
		/* No config file specified. Look for one where binary lives */
		if ((p = strrchr(argv[0], XPR_PATH_DELIM)) != 0) 
		{
			_snprintf(path, sizeof(path), "%.*s%s", (int) (p - argv[0]) + 1, argv[0], config_file);
			config_file = path;
			path[FILENAME_MAX - 1] = 0;
		}
	}

	fp = fopen(config_file, "r");
	/* If config file was set in command line and open failed, exit */
	if (fp == NULL && argv[i] != NULL && argv[i][0] != '+') 
	{
		fprintf(stderr, "cannot open config file \"%s\": %s\n", config_file, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (fp != NULL) 
	{
		xplDisplayMessage(xplMsgInfo, BAD_CAST "Loading web server config file \"%s\"...", config_file);
		/* Loop over the lines in config file */
		while (fgets(line, sizeof(line), fp) != NULL) 
		{
			line_no++;
			/* Ignore empty lines and comments */
			if (line[0] == '#' || line[0] == '\n')
				continue;
			if (sscanf(line, "%s %[^\r\n#]", opt, val) != 2) 
			{
				fprintf(stderr, "%s: line %d is invalid\n", config_file, (int) line_no);
				exit(EXIT_FAILURE);
			}
			if (mg_set_option(ctxt, opt, val) != 1)
				exit(EXIT_FAILURE);
		}
		fclose(fp);
	}
	/* Now pass through the command line options */
	for (i = 1; argv[i] != NULL && argv[i][0] == '-'; i += 2)
		mg_set_option(ctxt, &argv[i][1], argv[i + 1]);
}

xmlChar* getAppType(void)
{
	return BAD_CAST "web";
}

#ifdef _USE_TCMALLOC
char* XMLCALL tc_strdup(const char *src)
{
	char *ret;
	size_t len;
	if (!src)
		return NULL;
	len = strlen(src);
	ret = (char*) tc_malloc(len + 1);
	if (!ret)
		return NULL;
	memcpy(ret, src, len);
	*(ret+len) = 0;
	return ret;
}
#endif

int main(int argc, char* argv[])
{
	xplError err_code;
	xefStartupParams xef_params;
	xmlChar *xef_error_text;

	/* Запуск обвязки. MemSetup - сразу же, xmlMalloc вызывается практически везде */
#ifdef _USE_TCMALLOC 
	/* Безумству храбрых… */
	xmlMemSetup(tc_free, tc_malloc, tc_realloc, tc_strdup);
	xplDisplayMessage(xplMsgInfo, BAD_CAST "This is an experimental version using tcmalloc.");
#elif defined(_LEAK_DETECTION)
	xmlMemSetup(xmlMemFree, xmlMemMalloc, xmlMemRealloc, xmlMemoryStrdup);
#endif	
	xplDisplayMessage(xplMsgInfo, BAD_CAST "Starting supplementary layer...");
	xprParseCommandLine();
	//checkPid(); /* Это до всего остального */
	xprStartup(XPR_STARTSTOP_EVERYTHING);
	xmlInitParser();
	LIBXML_TEST_VERSION
		
	xplDisplayMessage(xplMsgInfo, BAD_CAST "Starting external libraries...");
	if (!xefStartup(&xef_params))
	{
		if (xef_params.error)
		{
			xef_error_text = xefGetErrorText(xef_params.error);
			xplDisplayMessage(xplMsgError, xef_error_text);
			xmlFree(xef_error_text);
			xefFreeErrorMessage(xef_params.error);
		} else
			xplDisplayMessage(xplMsgError, BAD_CAST "external libraries startup failed (unknown error)");
		xmlCleanupParser();
		xprShutdown(XPR_STARTSTOP_EVERYTHING);
		exit(-2);
	}

	/* Базовый каталог */
	/* ToDo URGENT xprGetProgramPath() */
	app_path = (xmlChar*) xmlMalloc(strlen(argv[0]) + 1);
	strcpy((char*) app_path, argv[0]);
	xmlChar *conf_path;
	/* Чудеса на разных версиях винды: argv[0] может оказаться только именем файла, даже без расширения. */
	char *fn_pos = strrchr((char*) app_path, XPR_PATH_DELIM);
	if (fn_pos)
	{
		fn_pos[1] = 0;
		conf_path = (xmlChar*) xmlMalloc(xmlStrlen(app_path) + 8);
		strcpy((char*) conf_path, (char*) app_path);
		strcat((char*) conf_path, "xpl.xml");
	} else 
		conf_path = xmlStrdup(BAD_CAST "xpl.xml");

	/* Инициализация движка XPL */
	xplDisplayMessage(xplMsgInfo, BAD_CAST "Starting XPL engine...");
	xplDisplayMessage(xplMsgInfo, BAD_CAST "Loading engine config file \"%s\"...", conf_path);
	err_code = xplInitParser(conf_path);
	if (err_code != XPL_ERR_NO_ERROR)
	{
		xplDisplayMessage(xplMsgError, BAD_CAST "error starting interpreter: \"%s\"", xplErrorToString(err_code));
		xefShutdown();
		xmlCleanupParser();
		xprShutdown(XPR_STARTSTOP_EVERYTHING);
		exit(EXIT_FAILURE);
	}
	xmlFree(conf_path);

	/* остатки болотца: синхронизация и хуки */
	lock_threads_event = XPR_CREATE_EVENT(NULL);
	XPR_SET_EVENT(lock_threads_event);
	xprInitSmartSemaphore(&worker_threads_semaphore, 0, 1);
	xplSetGlobalThreadSemaphore(&worker_threads_semaphore);
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	xplSetLockThreadsFunc(lockThreads);
	xprSetShutdownFunc(shutdownServer);
	xplSetGetAppTypeFunc(getAppType);
#ifdef _WIN32
	SetConsoleCtrlHandler(&win_signal_handler, TRUE);
#endif

	if ((ctx = mg_start()) == NULL) 
	{
		xplDisplayMessage(xplMsgError, BAD_CAST "Cannot initialize Mongoose context");
		xprShutdown(XPR_STARTSTOP_EVERYTHING);
		exit(EXIT_FAILURE);
	}
	if (!mg_get_option(ctx, "ports") && (mg_set_option(ctx, "ports", "8083") != 1))
	{
		xplDisplayMessage(xplMsgError, BAD_CAST "Cannot bind to listening port");
		xprShutdown(XPR_STARTSTOP_EVERYTHING);
		exit(EXIT_FAILURE);
	}
	process_command_line_arguments(ctx, argv); /* Здесь порт по умолчанию могут перекрыть */
	mg_set_uri_callback(ctx, "*.xpl", &serveXpl, ctx);
	xplDisplayMessage(xplMsgInfo, BAD_CAST "XPL web server based on Mongoose %s started on port(s) [%s], serving directory [%s]",
	    mg_version(),
	    mg_get_option(ctx, "ports"),
	    mg_get_option(ctx, "root"));
	fflush(stdout);
	/* Переключим корень приложения, перекрывая все настройки */
	app_path = BAD_CAST mg_get_option(ctx, "root");
	xplSetDocRoot(app_path);

	while (exit_flag == 0)
		xprSleep(100);
//	shutdownServer();
	xprSleep(1000);
	return (EXIT_SUCCESS);
}

