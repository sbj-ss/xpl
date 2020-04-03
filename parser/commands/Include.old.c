#include <commands/Include.h>
#include "Utils.h"
#include "ReszBuf.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <share.h>
#include <io.h>
#include "libxml/nanoftp.h"
#include "libxml/uri.h"
/* HTML Tidy */
#include "tidy/buffio.h"
#include "tidy/tidy.h"

#ifdef _WIN32
#include <Winhttp.h>
#endif

/* ��������� ������� ��������� ��������� ���������, �.�. ������� ����
   <xpl:include select="3*3"/> ����� ��������. �� �������������,
   ��� ����� ���� <xpl:value-of>.
 */
#undef _INCLUDE_SCALARS

void xplCmdIncludePrologue(xplCommandInfoPtr commandInfo)
{
}

xmlChar *encodingToStr(int encoding)
{
	switch (encoding)
	{
	case DETECTED_ENC_UNKNOWN: return cfgDefaultEncoding; break;
	case DETECTED_ENC_1251: return BAD_CAST "CP1251"; break;
	case DETECTED_ENC_UTF8: return BAD_CAST "utf-8"; break;
	case DETECTED_ENC_UTF16LE: return BAD_CAST "utf-16le"; break;
	case DETECTED_ENC_UTF16BE: return BAD_CAST "utf-16be"; break;
	case DETECTED_ENC_866: return BAD_CAST "cp866"; break;
	case DETECTED_ENC_KOI8: return BAD_CAST "KOI8-R"; break;
	default: return cfgDefaultEncoding;
	}
}

typedef enum 
{
	INC_FORMAT_UNKNOWN = -1,
	INC_FORMAT_XML = 1,
	INC_FORMAT_HTML,
	INC_FORMAT_TEXT,
	INC_FORMAT_BINARY_BASE64
} IncludeFormat;

typedef enum 
{
	INC_SOURCE_UNKNOWN = -1,
	INC_SOURCE_FILE = 1,
	INC_SOURCE_HTTP,
	INC_SOURCE_FTP,
	INC_SOURCE_MAX_NATIVE = INC_SOURCE_FTP
} IncludeSource;

IncludeFormat getIncludeFormat(xmlChar *format)
{
	if (!xmlStrcasecmp(format, BAD_CAST "xml"))
		return INC_FORMAT_XML;
	if (!xmlStrcasecmp(format, BAD_CAST "html"))
		return INC_FORMAT_HTML;
	if (!xmlStrcasecmp(format, BAD_CAST "text"))
		return INC_FORMAT_TEXT;
	if (!xmlStrcasecmp(format, BAD_CAST "binarybase64"))
		return INC_FORMAT_BINARY_BASE64;
	return INC_FORMAT_UNKNOWN;
}

IncludeSource getIncludeSource(xmlChar *source)
{
	if (xmlStrstr(source, BAD_CAST "file:///") == source)
		return INC_SOURCE_FILE;
	if (xmlStrstr(source, BAD_CAST "http://") == source)
		return INC_SOURCE_HTTP;
	return INC_SOURCE_UNKNOWN;
}

/* �������, ��� �� ����� ������ ��������� �� ������ ���� �� NULL */
xmlChar *uriFromLegacy(xmlChar *uri, xmlChar *file, BOOL abs_path, xmlChar *app_path)
{
	xmlChar *ret, *fn;
	if (uri)
		return xmlStrdup(uri);
	if (file)
	{
		if (abs_path)
			fn = file;
		else
			fn = xplFullFilename(file, app_path);
		ret = (xmlChar*) xmlMalloc(xmlStrlen(fn) + 9);
		strcpy((char*) ret, "file:///");
		strcat((char*) ret, (char*) fn);
		if (fn != file)
			xmlFree(fn);
		return ret;
	}
	return NULL;
}


/* ���������� ��������� ������������� �������� */
int fastDetectEncoding(char* content, size_t size)
{
#if 0
	FILE *fp = fopen("d:\\ss\\debug.txt", "w");
	fwrite(content, 1, size, fp);
	fclose(fp);
#endif
	if (size >= 2)
	{
		if ((*content == 0xFE) && (*(content+1) == 0xFF))
			return DETECTED_ENC_UTF16LE;
		if (!*content)
			return DETECTED_ENC_UTF16LE;
		if ((*content == 0xFF) && (*(content+1) == 0xFE))
			return DETECTED_ENC_UTF16BE;
		if (!*(content+1))
			return DETECTED_ENC_UTF16BE;
	}
	if (size >= 3)
	{
		if ((*content == 0xEF) && (*(content+1) == 0xBB) && (*(content+2) == 0xBF))
			return DETECTED_ENC_UTF8;
	}
	return detectEncoding(content, detectionSampleLen(content, size));
}

/* �� ����� ��������� ���������� size, �������� �� �������� */
xmlChar* fixEncoding(char* content, xmlChar *encoding, size_t *size)
{
	xmlChar *real_enc;
	char *ret;

	if (!encoding)
		return BAD_CAST content;
	if (!xmlStrcmp(encoding, BAD_CAST "auto")) {
		/* ����� �� ����� ������������ DEFAULT_ENC_DET_SAMPLE_LEN: ����� ���� ������� � ���� ����� ��� */
		real_enc = encodingToStr(fastDetectEncoding(content, *size));
	} else
		real_enc = encoding;
#if 0
	printf("*** Detected encoding: %s\n", real_enc);
#endif
	if (/*xmlStrcmp(encoding, real_enc) &&*/ xmlStrcmp(real_enc, BAD_CAST "utf-8"))
	{
		ret = NULL;
		iconv_string("utf-8", (const char*) real_enc, content, content + *size, &ret, size);
	} else
		ret = content;
	return BAD_CAST ret;
}

/* ������ � HTTP ���������� libxml2 ���������� � ��� � �������, ��� ����.
   ��� ���� ������ ������� ��������� ��� static � 100500 ��� �������� �����. 
   ���� ������ ��������������, �� ����� ��������������� ���.
 */
#define INT_RETRYTIMES 3

char* loadHTTPSource(xmlChar *url, xmlChar *encoding, size_t *size, xmlChar *uri_encoding, xmlChar *post_data)
{
/* ������������ ��� - (c) 2007-2009 Cheng Shi, shicheng107@hotmail.com */
	char *ret = NULL;
	char *escaped_uri = NULL;
	char szProxy[MAX_PATH];
	HINTERNET hSession;
	HINTERNET hConnect = NULL;
	HINTERNET hRequest = NULL;
	DWORD options;
	DWORD dwSize = 0, dwRead, dwTotal;
	unsigned int iRetryTimes = 0;
	WINHTTP_PROXY_INFO proxyInfo;
	URL_COMPONENTS urlComp;
	wchar_t wszHostName[MAX_PATH] = L"";
	wchar_t wszURLPath[MAX_PATH*5] = L"";
	wchar_t *wszRequestUrl = NULL;
	wchar_t *wszProxy = NULL;
	wchar_t *wszProxyUser = NULL;
	wchar_t *wszProxyPassword = NULL;
	wchar_t *wszHeader;
	size_t iconv_len;
	BOOL bOpResult = FALSE, bResponseSucceeded = FALSE;
	int status_code;
	ReszBufPtr buf = NULL;
	unsigned short zero = 0;

	*size = 0;
	if (!url)
		return NULL;

	escaped_uri = escapeURI(url);

	/* open the initial session */
	/* ToDo: maybe reuse at document level?.. */
	hSession = WinHttpOpen(L"Polaris XPL parser",  
			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME, 
			WINHTTP_NO_PROXY_BYPASS,
			0);
	if (!hSession)
		return (char*) xmlStrdup(BAD_CAST "<error>Cannot open WinHTTP session</error>");

	/* decode the URI */
	memset(&urlComp, 0, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(urlComp);
	urlComp.lpszHostName = wszHostName;
	urlComp.dwHostNameLength = MAX_PATH;
	urlComp.lpszUrlPath = wszURLPath;
	urlComp.dwUrlPathLength = MAX_PATH * 5;
	urlComp.dwSchemeLength = 1; /* None zero */
	iconv_string("utf-16le", "utf-8", escaped_uri, escaped_uri + strlen(escaped_uri), (char**) &wszRequestUrl, &iconv_len);
	if (!WinHttpCrackUrl(wszRequestUrl, 0/*iconv_len*/, 0, &urlComp))
	{
#if 0
		printf("WinHTTP CrackUrl error: %08x\n", GetLastError());
#endif
		ret = (char*) xmlStrdup(BAD_CAST "<error>The specified URI is incorrect</error>");
		goto done;
	}

	/* instantiate a connection (not really opening it yet) */
	hConnect = WinHttpConnect(hSession, urlComp.lpszHostName, urlComp.nPort, 0);
	if (!hConnect)
	{
		ret = (char*) xmlStrdup(BAD_CAST "<error>Cannot connect to the specified server</error>");
		goto done;
	}

	/* instantiate a request */
	options = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
	hRequest = WinHttpOpenRequest(hConnect, L"GET", urlComp.lpszUrlPath,
		NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,	options);
	if (!hRequest)
	{
		ret = (char*) xmlStrdup(BAD_CAST "<error>Cannot open WinHTTP request</error>");
		goto done;
	}

	/* If HTTPS, then client is very susceptable to invalid certificates */
	/* Easiest to accept anything for now */
	/* ToDo: security hole */
	if (TRUE && urlComp.nScheme == INTERNET_SCHEME_HTTPS)
	{
		options = SECURITY_FLAG_IGNORE_CERT_CN_INVALID
			| SECURITY_FLAG_IGNORE_CERT_DATE_INVALID
			| SECURITY_FLAG_IGNORE_UNKNOWN_CA;
		WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, (LPVOID) &options, sizeof(DWORD));
	}

	/* set up proxy if needed */
	if (cfgProxyServer)
	{
		memset(&proxyInfo, 0, sizeof(proxyInfo));
		proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
		sprintf(szProxy, "%s:%d", cfgProxyServer, cfgProxyPort?cfgProxyPort:80);
		iconv_string("utf-16le", "utf-8", (char*) szProxy, (char*) (szProxy + strlen(szProxy)), (char**) &wszProxy, NULL);
			proxyInfo.lpszProxy = wszProxy;
		if (!WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo)))
		{
			ret = (char*) xmlStrdup(BAD_CAST "<error>Cannot set up proxy</error>");
			goto done;
		}
		if (cfgProxyUser)
		{
			iconv_string("utf-16le", "utf-8", (char*) cfgProxyUser, (char*) (cfgProxyUser + xmlStrlen(cfgProxyUser)), (char**) &wszProxyUser, &iconv_len);
			if (!WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY_USERNAME, (LPVOID) wszProxyUser, (DWORD) iconv_len))
			{
				ret = (char*) xmlStrdup(BAD_CAST "<error>Cannot set up proxy user</error>");
				goto done;
			}
			if (cfgProxyPassword)
			{
				iconv_string("utf-16le", "utf-8", (char*) cfgProxyPassword, (char*)(cfgProxyPassword + xmlStrlen(cfgProxyPassword)), (char**) &wszProxyPassword, &iconv_len);
				if (!WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY_PASSWORD, (LPVOID) wszProxyPassword, (DWORD) iconv_len))
				{
					ret = (char*) xmlStrdup(BAD_CAST "<error>Cannot set up proxy password</error>");
					goto done;
				}
			} /* if(cfgProxyPassword) */
		} /* if(cfgProxyUser) */
	} /* if(cfgProxyServer) */

	/* Retry for several times if fails. */
	while (!bResponseSucceeded && iRetryTimes++ < INT_RETRYTIMES)
	{
		if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,	WINHTTP_NO_REQUEST_DATA, 0,	0, NULL))
		{
#if 0
			printf("WinHTTP error %08X", GetLastError());
#endif
			if (iRetryTimes == INT_RETRYTIMES)
			{
				ret = (char*) xmlStrdup(BAD_CAST "<error>Cannot send request</error>");
				goto done;
			} else
				continue; /* next try */
		}
#if 0
		if (m_pDataToSend != NULL)
		{
			DWORD dwWritten = 0;
			if (!WinHttpWriteData(hRequest, m_pDataToSend, m_dataToSendSize, &dwWritten))
			{
				if (iRetryTimes == INT_RETRYTIMES)
				{
					ret = (char*) xmlStrdup(BAD_CAST "<error>Cannot write data</error>");
					goto done;
				} else
					continue; /* next try */
			}
		} /* if data are present */
#endif
		if (!WinHttpReceiveResponse(hRequest, NULL))
		{
			if (iRetryTimes == INT_RETRYTIMES)
			{
				ret = (char*) xmlStrdup(BAD_CAST "<error>Cannot receive server response</error>");
				goto done;
			} else
				continue; /* next try */
		}
#if 0
		/* ������� ��������� */
		dwSize = 0;
		bOpResult = WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_ENCODING, WINHTTP_HEADER_NAME_BY_INDEX, NULL, &dwSize, WINHTTP_NO_HEADER_INDEX);
		if (bOpResult || (!bOpResult && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)))
		{
			wszHeader = (wchar_t*) xmlMalloc(dwSize*sizeof(wchar_t));
			if (wszHeader)
			{
				memset(wszHeader, 0, dwSize* sizeof(wchar_t));
				if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_ENCODING, WINHTTP_HEADER_NAME_BY_INDEX,	wszHeader, &dwSize, WINHTTP_NO_HEADER_INDEX))
				{
					six = wcsstr(wszHeader, L"Content-Length: ");
					if (six)
					{
						six += 15;
						//swscanf_s(six, L"%d", &content_length, dwSize - (six - wszHeader));
					}
				} /* if header queried successfully */
				xmlFree(wszHeader);
			} /* if the header is allocated */
		} /* query headers */
#endif
		/* ������� ��� ������� */
		/* ToDo: ��������� ��� */
		dwSize = 0;
		bOpResult = WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE, WINHTTP_HEADER_NAME_BY_INDEX, NULL, &dwSize, WINHTTP_NO_HEADER_INDEX);
		if (bOpResult || (!bOpResult && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)))
		{
			wszHeader = (wchar_t*) xmlMalloc(dwSize*sizeof(wchar_t));
			if (wszHeader != NULL)
			{
				memset(wszHeader, 0, dwSize* sizeof(wchar_t));
				if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE, WINHTTP_HEADER_NAME_BY_INDEX, wszHeader, &dwSize, WINHTTP_NO_HEADER_INDEX))
				{
					swscanf_s(wszHeader, L"%d", &status_code, dwSize);
				}
				xmlFree(wszHeader);
			}
		} /* if status queried */

		buf = createReszBufParams(16384, RESZ_BUF_GROW_DOUBLE, 0);
		dwTotal = 0;
		do
		{
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
			{
				if (iRetryTimes == INT_RETRYTIMES)
				{
					ret = (char*) xmlStrdup(BAD_CAST "<error>Cannot determine server response size (!?)</error>");
					goto done;
				} else
					continue; /* next try */
			}
			if (ensureReszBufSize(buf, dwSize) != RESZ_BUF_RESULT_OK)
			{
				ret = (char*) xmlStrdup(BAD_CAST "<error>Insufficient memory</error>");
				goto done;
			}
			ret = (char*) getReszBufPosition(buf);
			if (!WinHttpReadData(hRequest, ret, dwSize, &dwRead))
			{
				if (iRetryTimes == INT_RETRYTIMES)
				{
					ret = (char*) xmlStrdup(BAD_CAST "<error>Cannot read server response (!?)</error>");
					goto done;
				} else
						continue; /* next try */
			}
			advanceReszBufferPosition(buf, dwRead);
			dwTotal += dwRead;
		} while (dwSize > 0); /* reading cycle */
		bResponseSucceeded = TRUE;
	} // while
	addDataToReszBuf(buf, &zero, sizeof(zero));
	ret = (char*) detachReszBufContent(buf);
	*size = dwTotal;
done:
	if (!*size && ret)
		*size = strlen(ret);

	if (hSession) WinHttpCloseHandle(hSession);
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);

	if (wszRequestUrl) xmlFree(wszRequestUrl);
	if (wszProxy) xmlFree(wszProxy);
	if (wszProxyUser) xmlFree(wszProxyUser);
	if (wszProxyPassword) xmlFree(wszProxyPassword);

	if (escaped_uri) xmlFree(escaped_uri);
	if (buf) freeReszBuf(buf);

	return ret;
}

char* loadLocalFile(xmlChar *filename, xmlChar *encoding, size_t *size)
{
	char *ret;
	XPR_FS_CHAR *full_path = NULL;
	int fd, open_result;
	struct _stat32 stat;

	xprConvertSlashes(filename);
	iconv_string(FS_ENCODING, "utf-8", (char*) filename + 8, (char*) filename + xmlStrlen(filename), (char**) &full_path, NULL);
	/* ��� ����� _O_BINARY ������ �������� ����� *SCRATCH* */
	open_result = XPR_FILE_SOPEN(&fd, full_path, _O_BINARY | _O_RDONLY, _SH_DENYNO, 0);
	xmlFree(full_path);
	if (open_result)
		return NULL;
	/* ToDo: ����� ������, ������ ������� */
	_fstat32(fd, &stat);
	*size = stat.st_size;
	ret = (char*) xmlMalloc(*size + 2);
	if (!ret)
		return NULL;
	(void) _read(fd, ret, *size);
	*(ret + *size) = *(ret + *size + 1) = 0;
	_close(fd);
	return ret;
}

xmlChar* tidyHtmlDoc(xmlChar *src, size_t *size)
{
	TidyBuffer output = {0};
	TidyBuffer errbuf = {0};
	TidyDoc tdoc;
	xmlChar *ret = NULL;

	if (!(tdoc = tidyCreate()))
		return NULL;
	if (!tidyOptSetBool(tdoc, TidyXhtmlOut, yes)) /* ����, �� xml, � xhtml */
		goto done;
	if (!tidyOptSetBool(tdoc, TidyNumEntities, yes))
		goto done;
	if (!tidyOptSetBool(tdoc, TidyForceOutput, yes))
		goto done;
	if (tidySetCharEncoding(tdoc, "utf8"))
		goto done;
	if (tidySetErrorBuffer(tdoc, &errbuf) < 0)
		goto done;
	if (tidyParseString(tdoc, (char*) src) < 0)
		goto done;
	if (tidyCleanAndRepair(tdoc) < 0)
		goto done;
	if (cfgPrintTidyInfo)
	{
		tidyRunDiagnostics(tdoc);
#if 0
		printf((char*) errbuf.bp);
#endif
	}
	if (tidySaveBuffer(tdoc, &output) < 0)
		goto done;
	tidyBufPutByte(&output, 0); /* ensure NULL-termination */
#if 0
	tidySaveFile(tdoc, "d:\\ss\\debug.xml");
#endif
	ret = BAD_CAST output.bp;
	*size = output.size - 1;
	tidyBufDetach(&output);
done:
	tidyBufFree(&output);
	tidyBufFree(&errbuf);
	if (tdoc) tidyRelease(tdoc);
	return ret;
}

/* ��������� ������, �� ��������� ��� ��������� �����, � ���� ������.
   � ������ ������ ������������� *error � ���������� NULL. */
xmlChar* loadExtData(
		xmlChar *uri,				/* ������ ����� */
		IncludeFormat format,		/* � ����� ������� */
		IncludeSource src,			/* �� ������ ���������. ���� "����������" - ���������������. */
		xmlChar *encoding,			/* ��������� */
		xplCommandInfoPtr info,		/* ��� ��������� ������ */
		xmlNodePtr *error,			/* ��������� �� ��������������� ������ */
		size_t *size				/* ������ ���������� ������ */
)
{
	char *raw_content;
	xmlChar *fixed_content, *tidy_content;
	size_t tidy_size;

	if (src == INC_SOURCE_UNKNOWN)
		src = getIncludeSource(uri);
	if (src == INC_SOURCE_UNKNOWN)
	{
		*error = xplCreateErrorNode(info, BAD_CAST "invalid protocol in URI \"%s\"", uri);
		return NULL;
	}
	switch (src)
	{
	case INC_SOURCE_FILE: 
		raw_content = loadLocalFile(uri, encoding, size);
		break;
	case INC_SOURCE_HTTP:
		raw_content = loadHTTPSource(uri, encoding, size);
		break;
	default:
		DISPLAY_INTERNAL_ERROR_MESSAGE
		*error = xplCreateErrorNode(info, BAD_CAST "unknown source protocol (developer's error?)");
		return NULL;
	}
	if (!raw_content)
	{
		*error = xplCreateErrorNode(info, BAD_CAST "cannot load the specified URI \"%s\"", uri);
		return NULL;
	}
	fixed_content = fixEncoding(raw_content, encoding, size);
	if (fixed_content != BAD_CAST raw_content)
		xmlFree(raw_content);
/*printf("%s %d\n", uri, xmlStrlen(fixed_content));*/
#if 0
	FILE *fp = fopen("d:\\ss\\debug_enc.txt", "w");
	fwrite(fixed_content, 1, *size, fp);
	fclose(fp);
#endif
	if (format == INC_FORMAT_HTML)
	{
		/* ToDo: error checking */
		tidy_content = tidyHtmlDoc(fixed_content, size);
		xmlFree(fixed_content);
		fixed_content = tidy_content;
	} else if (format == INC_FORMAT_BINARY_BASE64) {
#pragma warning(push)
#pragma warning(disable: 4244)
		tidy_size = *size*4.0/3 + 3;
#pragma warning(pop)
		tidy_content = (xmlChar*) xmlMalloc(tidy_size);
		if (!tidy_content)
		{
			*error = xplCreateErrorNode(info, BAD_CAST "insufficient memory");
			xmlFree(fixed_content);
			return NULL;
		}
		base64encode(fixed_content, *size, (char*) tidy_content, tidy_size);
		xmlFree(fixed_content);
		fixed_content = tidy_content;
		*size = tidy_size;
	}
	return fixed_content;
}

xmlDocPtr loadExtDoc(xmlChar *uri, IncludeFormat format, xmlChar *encoding, xplCommandInfoPtr info, xmlNodePtr *error)
{
	IncludeSource src;
	xmlChar *doc_src, *err_txt;
	size_t doc_src_size;
	xmlDocPtr ret;
	xmlNodePtr txt;

	src = getIncludeSource(uri);
	if (src == INC_SOURCE_UNKNOWN)
	{
		*error = xplCreateErrorNode(info, BAD_CAST "invalid protocol in URI \"%s\"", uri);
		return NULL;
	}
	/* ���������� ������� */
	if (format == INC_FORMAT_XML)
	{
		if (src > INC_SOURCE_MAX_NATIVE)
		{
			*error = xplCreateErrorNode(info, BAD_CAST "protocols other than file, ftp and http are unsupported for native XML documents");
			return NULL;
		}
		if (src == INC_SOURCE_FILE)
			xprConvertSlashes(uri);
		ret = xmlParseFile((char*) uri);
		if (!ret)
		{
			err_txt = getLastLibxmlError();
			*error = xplCreateErrorNode(info, BAD_CAST "cannot load document, DOM parser error \"%s\"", err_txt);
			xmlFree(err_txt);
		}
		return ret;
	}
	/* �����, ������� ������ ������ */
	doc_src = loadExtData(uri, format, src, encoding, info, error, &doc_src_size);
	if (!doc_src)
		return NULL;

	switch(format)
	{
	case INC_FORMAT_TEXT:
	case INC_FORMAT_BINARY_BASE64:
		/* �������� ������������ ��������, � ������ ������ ��� ��������� */
		ret = xmlNewDoc(BAD_CAST "1.0");
		txt = xmlNewDocText(ret, NULL);
		txt->content = doc_src;
		setChildren((xmlNodePtr) ret, txt);
		break;
	case INC_FORMAT_HTML:
		ret = xmlParseMemory((char*) doc_src, doc_src_size);
		xmlFree(doc_src);
		break;
	default:
		DISPLAY_INTERNAL_ERROR_MESSAGE
		*error = xplCreateErrorNode(info, BAD_CAST "unknown format, perhaps a bug");
		return NULL;
	}
	return ret;
}

void xplCmdIncludeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define SELECT_ATTR (BAD_CAST "select")
#define SOURCE_ATTR (BAD_CAST "source")
#define FILE_ATTR (BAD_CAST "file")
#define URI_ATTR (BAD_CAST "uri")
#define REPEAT_ATTR (BAD_CAST "repeat")
#define RESPONSE_TAG_NAME_ATTR (BAD_CAST "responsetagname")
#define AS_TEXT_ATTR (BAD_CAST "astext")
#define ENCODING_ATTR (BAD_CAST "encoding")
#define URI_ENCODING_ATTR (BAD_CAST "uriencoding")
#define ABS_PATH_ATTR (BAD_CAST "abspath")
#define FORMAT_ATTR (BAD_CAST "format")

	/* ��������� ������� */
	xmlChar *select_attr = NULL;		/* XPath-��������� ������� */
	xmlChar *file_attr = NULL;			/* ��� ����������� ����� */
	xmlChar *uri_attr = NULL;			/* URI �������� ��������� */
	xmlChar *repeat_attr = NULL;		/* ��������� �� ��������� ������� (��) */
	xmlChar *res_tag_name_attr = NULL;	/* ����� ��� ��� ����� �������� ������ */
	xmlChar *encoding_attr = NULL;		/* ��������� �������� HTML/txt */
	xmlChar *uri_encoding_attr = NULL;	/* ��������� ���� � ��������� ������� (http://a.ru?q=�����) */
	xmlChar *abs_path_attr = NULL;		/* ���� � ����� �������� ���������� (���) */
	xmlChar *as_text_attr = NULL;		/* ������� ��������� ���� */
	xmlChar *format_attr = NULL;
	/* �������������� ��������� */
	BOOL repeat = TRUE;
	BOOL abs_path = FALSE;
	BOOL as_text = FALSE;
	IncludeFormat format = INC_FORMAT_XML;
	/* ���������� ���������� */
	xmlChar *uri = NULL;				/* ����� �������� ��������� */
	char *encoded_uri = NULL;			/* �� �� � �������� ��������� */
	xmlDocPtr ext_doc = NULL;			/* ���������� ������� �������� */
	xmlNodePtr ret = NULL;				/* ������������ ����� ����� */
	xmlXPathObjectPtr sel = NULL;		/* ��������� XPath-������� */
	int i;								/* ������� ��� ������ */
	xmlNodePtr tail = NULL;				/* ����� ������������ ������ �������� */
	xmlNodePtr sibling;					/* ������� �� ���������� */
	xmlNodePtr error = NULL;			/* ������ �������� ��������� */
	xmlChar *content;					/* ���������� ������������� ����� */
	size_t content_size;				/* ������ ����������� ������ */
	xmlNodePtr prnt, copy;				/* ��������� ��� ����������� */

	/* ������� ��� ���������, �������� "����������" � ����������� */
	select_attr = xmlGetNoNsProp(commandInfo->element, SELECT_ATTR);
	if (!select_attr)
		select_attr = xmlGetNoNsProp(commandInfo->element, SOURCE_ATTR);
	file_attr = xmlGetNoNsProp(commandInfo->element, FILE_ATTR);
	uri_attr = xmlGetNoNsProp(commandInfo->element, URI_ATTR);
	repeat_attr = xmlGetNoNsProp(commandInfo->element, REPEAT_ATTR);
	if (repeat_attr && !xmlStrcasecmp(repeat_attr, BAD_CAST "false"))
		repeat = FALSE;
	res_tag_name_attr = xmlGetNoNsProp(commandInfo->element, RESPONSE_TAG_NAME_ATTR);
	encoding_attr = xmlGetNoNsProp(commandInfo->element, ENCODING_ATTR);
	uri_encoding_attr = xmlGetNoNsProp(commandInfo->element, URI_ENCODING_ATTR);
	abs_path_attr = xmlGetNoNsProp(commandInfo->element, ABS_PATH_ATTR);
	if (abs_path_attr && !xmlStrcasecmp(abs_path_attr, BAD_CAST "true"))
		abs_path = TRUE;
	as_text_attr = xmlGetNoNsProp(commandInfo->element, AS_TEXT_ATTR);
	if (as_text_attr && !xmlStrcasecmp(as_text_attr, BAD_CAST "true"))
		as_text = TRUE;
	format_attr = xmlGetNoNsProp(commandInfo->element, FORMAT_ATTR);
	if (format_attr)
	{
		format = getIncludeFormat(format_attr);
		if (format == INC_FORMAT_UNKNOWN)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo, BAD_CAST "invalid format attribute value \"%s\"", format_attr), TRUE, TRUE)
			goto done;
		}
	}
	/* ����� �������� �� �������������, ��� ������� ���� ���������� �� ����, � ���. ���������� */
	if (file_attr || uri_attr)
	{
		/* �������� �� �������� */
		if (file_attr && uri_attr)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo, BAD_CAST "file and uri specified simultaneously"), TRUE, TRUE)
			goto done; 
		}
		if (abs_path_attr && uri_attr)
		{
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo, BAD_CAST "abspath and uri specified simultaneously"), TRUE, TRUE)
				goto done; 
		}
		uri = uriFromLegacy(uri_attr, file_attr, abs_path, commandInfo->document->app_path);
		if (uri_encoding_attr)
		{
			if (iconv_string((char*) uri_encoding_attr, "utf-8", (char*) uri, (char*) uri+xmlStrlen(uri), &encoded_uri, NULL))
			{
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo, BAD_CAST "cannot convert document uri to encoding \"%s\"", uri_encoding_attr), TRUE, TRUE);
				goto done;
			}
		} else
			encoded_uri = (char*) uri;
	}
	/* � ��������� �����������, ������ ����� ����� */
	if (encoded_uri)
	{
		if ((format != INC_FORMAT_XML) && as_text && !select_attr) /* �� ���� ������ �������. ����� ��������� ����. */
		{
			content = loadExtData(BAD_CAST encoded_uri, format, INC_SOURCE_UNKNOWN, encoding_attr, commandInfo, &error, &content_size);
			if (content)
			{
				ret = xmlNewDocText(commandInfo->element->doc, NULL);
				ret->content = content;
				ASSIGN_RESULT(ret, repeat, TRUE)
			}
			else
				ASSIGN_RESULT(error, TRUE, TRUE)
			goto done;
		}
		ext_doc = loadExtDoc(BAD_CAST encoded_uri, format, encoding_attr, commandInfo, &error);
		if (!ext_doc)
		{
			ASSIGN_RESULT(error, TRUE, TRUE);
			goto done;
		}
	}
	if (!uri) /* ��������� ������� */
		sibling = commandInfo->element;
	else if (select_attr) {
		sibling = ext_doc->children;
		while (sibling->type == XML_COMMENT_NODE && sibling)
			sibling = sibling->next; /* ��������� ����������� �� ��������� ��-�� */
	} else
		sibling = (xmlNodePtr) ext_doc;

	if (select_attr)
	{
		sel = xplSelectNodes(commandInfo, sibling, select_attr);
		if (sel)
		{
			if (sel->type == XPATH_NODESET)
			{
				if (sel->nodesetval)
				{
					for (i = 0; i < sel->nodesetval->nodeNr; i++)
					{
						sibling = sel->nodesetval->nodeTab[i];
						prnt = sibling->parent;
						sibling->parent = NULL;
						copy = cloneAttrAsText(sibling, commandInfo->element);
						sibling->parent = prnt;
						if (!tail)
							tail = copy;
						else
							tail = xmlAddNextSibling(tail, copy);
						if (!ret)
							ret = tail;
					}
				}
			} else if (sel->type != XPATH_UNDEFINED) {
#ifdef _INCLUDE_SCALARS
				ret = xmlNewDocText(commandInfo->document->document, NULL);
				ret->content = xmlXPathCastToString(sel);
#else
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo, BAD_CAST "select XPath expression (%s) evaluated to non-nodeset value", select_attr), TRUE, TRUE);
				goto done;
#endif
			} else {
				ASSIGN_RESULT(xplCreateErrorNode(commandInfo, BAD_CAST "select XPath expression (%s) evaluated to undef", select_attr), TRUE, TRUE);
				goto done;
			}
		} else {
			ASSIGN_RESULT(xplCreateErrorNode(commandInfo, BAD_CAST "invalid select XPath expression (%s)", select_attr), TRUE, TRUE);
			goto done;
		}
	} else if (!ret) {
		ret = detachContent(sibling);
		if (ext_doc)
		{
			xmlSetListDoc(ret, commandInfo->element->doc);
			ext_doc->intSubset = NULL;
		}
		downshiftNodeListNsDef(ret, commandInfo->element->nsDef);
	}
	if (res_tag_name_attr)
	{
		sibling = ret;
		while (sibling)
		{
			if (sibling->type == XML_ELEMENT_NODE)
				xmlNodeSetName(sibling, res_tag_name_attr);
			/* ToDo: ��������� ��������� ����?.. */
			sibling = sibling->next;
		}
	}

	ASSIGN_RESULT(ret, repeat, TRUE);
done:
	if (select_attr) xmlFree(select_attr);
	if (file_attr) xmlFree(file_attr);
	if (uri_attr) xmlFree(uri_attr);
	if (repeat_attr) xmlFree(repeat_attr);
	if (res_tag_name_attr) xmlFree(res_tag_name_attr);
	if (encoding_attr) xmlFree(encoding_attr);
	if (uri_encoding_attr) xmlFree(uri_encoding_attr);
	if (abs_path_attr) xmlFree(abs_path_attr);
	if (as_text_attr) xmlFree(as_text_attr);
	if (format_attr) xmlFree(format_attr);

	if ((BAD_CAST encoded_uri) != uri) xmlFree(encoded_uri);
	if (uri) xmlFree(uri);
	if (sel) xmlXPathFreeObject(sel);
	if (ext_doc) xmlFreeDoc(ext_doc);
}

xplCommand xplIncludeCommand = { xplCmdIncludePrologue, xplCmdIncludeEpilogue };

