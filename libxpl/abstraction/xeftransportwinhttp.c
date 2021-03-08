#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#include <libxml/tree.h>
#include <libxpl/abstraction/xef.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplstring.h>

bool xefStartupTransport(xefStartupParamsPtr params)
{
	UNUSED_PARAM(params);

	return true;
}

void xefShutdownTransport(void)
{
}

static xmlChar* xefCreateTransportErrorMessage(xmlChar *fmt, ...)
{
	WCHAR *buffer;
	xmlChar *sys_error = NULL, *formatted, *ret;
	va_list args;

	va_start(args, fmt);
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, (LPWSTR) &buffer, 0, NULL);
	if (!buffer)
		return NULL;
	xstrIconvString("utf-8", "utf-16le", (char*) buffer, (char*) buffer + wcslen(buffer)*sizeof(WCHAR), (char**) &sys_error, NULL);
	LocalFree(buffer);
	if (!sys_error)
		return NULL;
	formatted = xplVFormatMessage(fmt, args);
	if (!formatted)
		return NULL;
	ret = xplFormatMessage(BAD_CAST "%s: %s", formatted, sys_error);
	XPL_FREE(sys_error);
	XPL_FREE(formatted);
	return ret;
}

#define INT_RETRYTIMES 3

bool xefFetchDocument(xefFetchDocumentParamsPtr params)
{
	/* original code (c) 2007-2009 Cheng Shi, shicheng107@hotmail.com */
	HINTERNET hSession;
	HINTERNET hConnect = NULL;
	HINTERNET hRequest = NULL;
	WCHAR *wszRequestUrl = NULL;
	URL_COMPONENTSW urlComp;
	char *szProxy = NULL;
	WINHTTP_PROXY_INFOW proxyInfo;
	WCHAR *wszProxyUser = NULL;
	WCHAR *wszProxyPassword = NULL;
	WCHAR *wszHeader;
	WCHAR *wsz_extra_headers;
	size_t iconv_len;
	bool bOpResult = false, bResponseSucceeded = false;
	unsigned int iRetryTimes = 0;
	int status_code;
	DWORD extra_query_len;
	DWORD options;
	DWORD dwSize = 0, dwRead, dwTotal;
	xmlBufferPtr buf = NULL;
	unsigned short zero = 0;

	params->document = NULL;
	params->document_size = 0;
	if (!params->uri)
	{
		params->error = BAD_CAST XPL_STRDUP("empty URI");
		return false;
	}

	/* ToDo: maybe reuse at document level?.. */
	/* ToDo: configurable agent */
	hSession = WinHttpOpen(L"Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0)",  
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME, 
		WINHTTP_NO_PROXY_BYPASS,
		0);
	if (!hSession)
	{
		params->error = xefCreateTransportErrorMessage(BAD_CAST "cannot open WinHTTP session");
		return false;
	}
	memset(&urlComp, 0, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(urlComp);
	urlComp.dwHostNameLength = 256;
	urlComp.lpszHostName = (WCHAR*) XPL_MALLOC((size_t) urlComp.dwHostNameLength);
	urlComp.dwUrlPathLength = 1024*2;
	urlComp.lpszUrlPath = (WCHAR*) XPL_MALLOC((size_t) urlComp.dwUrlPathLength);
	urlComp.dwExtraInfoLength = 1024*32;
	urlComp.lpszExtraInfo = (WCHAR*) XPL_MALLOC((size_t) urlComp.dwExtraInfoLength); /* use POST for larger data volumes */
	xstrIconvString("utf-16le", "utf-8", (char*) params->uri, (char*) params->uri + xmlStrlen(params->uri), (char**) &wszRequestUrl, NULL);
	if (!WinHttpCrackUrl(wszRequestUrl, 0, 0, &urlComp))
	{
		params->error = xefCreateTransportErrorMessage(BAD_CAST "the specified URI \"%s\" is incorrect", params->uri);
		goto done;
	}

	/* instantiate a connection (not really opening it yet) */
	hConnect = WinHttpConnect(hSession, urlComp.lpszHostName, urlComp.nPort, 0);
	if (!hConnect)
	{
		params->error = xefCreateTransportErrorMessage(BAD_CAST "cannot connect to %s", params->uri);
		goto done;
	}

	/* instantiate a request */
	options = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
	wcscpy(wszRequestUrl, urlComp.lpszUrlPath);
	wcscat(wszRequestUrl, urlComp.lpszExtraInfo);
	hRequest = WinHttpOpenRequest(hConnect, params->extra_query? L"POST": L"GET", wszRequestUrl,
		NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,	options);
	if (!hRequest)
	{
		params->error = xefCreateTransportErrorMessage(BAD_CAST "cannot open WinHTTP request to %s", params->uri);
		goto done;
	}

	/* If HTTPS, then client is very susceptable to invalid certificates */
	/* Easiest to accept anything for now */
	/* ToDo: security hole */
	if (urlComp.nScheme == INTERNET_SCHEME_HTTPS)
	{
		options = SECURITY_FLAG_IGNORE_CERT_CN_INVALID
			| SECURITY_FLAG_IGNORE_CERT_DATE_INVALID
			| SECURITY_FLAG_IGNORE_UNKNOWN_CA;
		WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, (LPVOID) &options, sizeof(DWORD));
	}

	/* set up proxy if needed */
	memset(&proxyInfo, 0, sizeof(proxyInfo));
	if (cfgProxyServer)
	{
		proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
		szProxy = (char*) xplFormatMessage(BAD_CAST "%s:%d", cfgProxyServer, cfgProxyPort?cfgProxyPort:80);
		xstrIconvString("utf-16le", "utf-8", (char*) szProxy, (char*) (szProxy + strlen(szProxy)), (char**) &proxyInfo.lpszProxy, NULL);
		if (!WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo)))
		{
			params->error = xefCreateTransportErrorMessage(BAD_CAST "cannot set up proxy %s:%d", cfgProxyServer, cfgProxyPort);
			goto done;
		}
		if (cfgProxyUser)
		{
			xstrIconvString("utf-16le", "utf-8", (char*) cfgProxyUser, (char*) (cfgProxyUser + xmlStrlen(cfgProxyUser)), (char**) &wszProxyUser, &iconv_len);
			if (!WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY_USERNAME, (LPVOID) wszProxyUser, (DWORD) iconv_len))
			{
				params->error = xefCreateTransportErrorMessage(BAD_CAST "cannot set up proxy user %s", cfgProxyUser);
				goto done;
			}
			if (cfgProxyPassword)
			{
				xstrIconvString("utf-16le", "utf-8", (char*) cfgProxyPassword, (char*)(cfgProxyPassword + xmlStrlen(cfgProxyPassword)), (char**) &wszProxyPassword, &iconv_len);
				if (!WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY_PASSWORD, (LPVOID) wszProxyPassword, (DWORD) iconv_len))
				{
					params->error = xefCreateTransportErrorMessage(BAD_CAST "cannot set up proxy password");
					goto done;
				}
			} /* if(cfgProxyPassword) */
		} /* if(cfgProxyUser) */
	} /* if(cfgProxyServer) */

	if (params->extra_query) /* use POST */
	{
		extra_query_len = xmlStrlen(params->extra_query);
		wsz_extra_headers = L"Content-Type: application/x-www-form-urlencoded";
	} else {
		wsz_extra_headers = NULL;
		extra_query_len = 0;
	}

	/* Retry for several times if fails. TODO: configure */
	while (!bResponseSucceeded && iRetryTimes++ < INT_RETRYTIMES)
	{
		if (!WinHttpSendRequest(hRequest, wsz_extra_headers, -1, params->extra_query, extra_query_len, extra_query_len, (DWORD_PTR) NULL))
		{
			if (iRetryTimes == INT_RETRYTIMES)
			{
				params->error = xefCreateTransportErrorMessage(BAD_CAST "cannot send request to %s", params->uri);
				goto done;
			} else
				continue; /* next try */
		}
		if (!WinHttpReceiveResponse(hRequest, NULL))
		{
			if (iRetryTimes == INT_RETRYTIMES)
			{
				params->error = xefCreateTransportErrorMessage(BAD_CAST "cannot receive server response from %s", params->uri);
				goto done;
			} else
				continue; /* next try */
		}
#if 0
		/* ToDo: find encoding */
		dwSize = 0;
		bOpResult = WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_ENCODING, WINHTTP_HEADER_NAME_BY_INDEX, NULL, &dwSize, WINHTTP_NO_HEADER_INDEX);
		if (bOpResult || (!bOpResult && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)))
		{
			wszHeader = (WCHAR*) XPL_MALLOC(dwSize*sizeof(WCHAR));
			if (wszHeader)
			{
				memset(wszHeader, 0, dwSize* sizeof(WCHAR));
				if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_ENCODING, WINHTTP_HEADER_NAME_BY_INDEX,	wszHeader, &dwSize, WINHTTP_NO_HEADER_INDEX))
				{
					six = wcsstr(wszHeader, L"Content-Length: ");
					if (six)
					{
						six += 15;
						//swscanf_s(six, L"%d", &content_length, dwSize - (six - wszHeader));
					}
				} /* if header queried successfully */
				XPL_FREE(wszHeader);
			} /* if the header is allocated */
		} /* query headers */
#endif
		/* temp stub */
		params->encoding = NULL;
		/* ToDo: maybe check status code? */
		dwSize = 0;
		bOpResult = WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE, WINHTTP_HEADER_NAME_BY_INDEX, NULL, &dwSize, WINHTTP_NO_HEADER_INDEX);
		if (bOpResult || (!bOpResult && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)))
		{
			wszHeader = (WCHAR*) XPL_MALLOC((size_t) dwSize*sizeof(WCHAR));
			if (wszHeader != NULL)
			{
				memset(wszHeader, 0, (size_t) dwSize* sizeof(WCHAR));
				if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE, WINHTTP_HEADER_NAME_BY_INDEX, wszHeader, &dwSize, WINHTTP_NO_HEADER_INDEX))
				{
					swscanf_s(wszHeader, L"%d", &status_code, dwSize);
					params->status_code = status_code;
				}
				XPL_FREE(wszHeader);
			}
		} /* if status queried */

		buf = xmlBufferCreateSize(16384);
		dwTotal = 0;
		do
		{
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
			{
				if (iRetryTimes == INT_RETRYTIMES)
				{
					params->error = xefCreateTransportErrorMessage(BAD_CAST "cannot determine server response size (!?)");
					goto done;
				} else
					continue; /* next try */
			}
			if (xmlBufferGrow(buf, (size_t) dwSize) < 0)
			{
				params->error = BAD_CAST XPL_STRDUP("insufficient memory");
				goto done;
			}
			params->document = xmlBufferContent(buf) + xmlBufferLength(buf);
			if (!WinHttpReadData(hRequest, params->document, dwSize, &dwRead))
			{
				if (iRetryTimes == INT_RETRYTIMES)
				{
					params->error = xefCreateTransportErrorMessage(BAD_CAST "cannot read server response (!?)");
					goto done;
				} else
					continue; /* next try */
			}
			xmlBufferAdd(buf, params->document, (size_t) dwRead); // TODO this is ineffective
			dwTotal += dwRead;
		} while (dwSize > 0); /* reading cycle */
		bResponseSucceeded = true;
	} // while
	xmlBufferAdd(buf, BAD_CAST &zero, sizeof(zero));
	params->document = (xmlChar*) xmlBufferDetach(buf);
	params->document_size = (size_t) dwTotal;
	params->error = NULL;
	/* ToDo: set this field */
	params->real_uri = NULL;
done:
	if (hSession) WinHttpCloseHandle(hSession);
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);

	if (wszRequestUrl) XPL_FREE(wszRequestUrl);
	if (urlComp.lpszHostName) XPL_FREE(urlComp.lpszHostName);
	if (urlComp.lpszUrlPath) XPL_FREE(urlComp.lpszUrlPath);
	if (urlComp.lpszExtraInfo) XPL_FREE(urlComp.lpszExtraInfo);
	if (szProxy) XPL_FREE(szProxy);
	if (proxyInfo.lpszProxy) XPL_FREE(proxyInfo.lpszProxy);
	if (wszProxyUser) XPL_FREE(wszProxyUser);
	if (wszProxyPassword) XPL_FREE(wszProxyPassword);

	if (buf) xmlBufferFree(buf);

	return (params->error)? false: true;
}

void xefFetchParamsClear(xefFetchDocumentParamsPtr params)
{
	if (params->document)
		xmlFree(params->document);
	if (params->encoding)
		xmlFree(params->encoding);
	if (params->real_uri)
		xmlFree(params->real_uri);
}

