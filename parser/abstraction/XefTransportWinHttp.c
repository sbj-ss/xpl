#include "Common.h"
#include <Winhttp.h>
#include "ReszBuf.h"
#include "Utils.h"
#include "Core.h"
#include "abstraction/ExtFeatures.h"
#include "abstraction/XefInternal.h"

XEF_STARTUP_PROTO(Transport)
{
	return true;
}

XEF_SHUTDOWN_PROTO(Transport)
{

}

typedef struct _xefTransportErrorMessage
{
	xefErrorMessageHeader header;
	xmlChar *message_text;
	DWORD last_error;
} xefTransportErrorMessage, *xefTransportErrorMessagePtr;

xefErrorMessagePtr xefCreateTransportErrorMessage(xmlChar *src)
{
	xefTransportErrorMessagePtr ret = (xefTransportErrorMessagePtr) xmlMalloc(sizeof(xefTransportErrorMessage));
	if (!ret)
		return NULL;
	ret->header.subsystem = XEF_SUBSYSTEM_TRANSPORT;
	ret->last_error = GetLastError();
	ret->message_text = src;
	return (xefErrorMessagePtr) ret;
}

XEF_GET_ERROR_TEXT_PROTO(Transport)
{
	xmlChar *sys_error = NULL, *ret;
	wchar_t *buffer;
	xefTransportErrorMessagePtr real_msg = (xefTransportErrorMessagePtr) msg;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, real_msg->last_error, 0, (LPWSTR) &buffer, 0, NULL);
	if (buffer)
	{
		iconv_string("utf-8", "utf-16le", (char*) buffer, (char*) buffer + wcslen(buffer)*sizeof(wchar_t), (char**) &sys_error, NULL);
		LocalFree(buffer);
		ret = xplFormatMessage("%s: %s", real_msg->message_text, sys_error);
		xmlFree(sys_error);
	} else
		ret = xmlStrdup(real_msg->message_text);
	return ret;
}

XEF_FREE_ERROR_MESSAGE_PROTO(Transport)
{
	xefTransportErrorMessagePtr real_msg = (xefTransportErrorMessagePtr) msg;
	if (!msg)
		return;
	if (real_msg->message_text) 
		xmlFree(real_msg->message_text);
	xmlFree(msg);
}

#define INT_RETRYTIMES 3

bool xefFetchDocument(xefFetchDocumentParamsPtr params)
{
	/* ������������ ��� - (c) 2007-2009 Cheng Shi, shicheng107@hotmail.com */
	HINTERNET hSession;
	HINTERNET hConnect = NULL;
	HINTERNET hRequest = NULL;
	wchar_t *wszRequestUrl = NULL;
	URL_COMPONENTSW urlComp;
	char *szProxy = NULL;
	WINHTTP_PROXY_INFOW proxyInfo;
	wchar_t *wszProxyUser = NULL;
	wchar_t *wszProxyPassword = NULL;
	wchar_t *wszHeader;
	wchar_t *wsz_extra_headers;
	size_t iconv_len;
	bool bOpResult = false, bResponseSucceeded = false;
	unsigned int iRetryTimes = 0;
	int status_code;
	DWORD extra_query_len;
	DWORD options;
	DWORD dwSize = 0, dwRead, dwTotal;
	ReszBufPtr buf = NULL;
	unsigned short zero = 0;

	params->document = NULL;
	params->document_size = 0;
	if (!params->uri)
	{
		params->error = xefCreateCommonErrorMessage(BAD_CAST "empty URI");
		return false;
	}

	/* ToDo: maybe reuse at document level?.. */
	/* �������� ��� ������ ��������� */
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
	urlComp.lpszHostName = (wchar_t*) xmlMalloc((size_t) urlComp.dwHostNameLength);
	urlComp.dwUrlPathLength = 1024*2;
	urlComp.lpszUrlPath = (wchar_t*) xmlMalloc((size_t) urlComp.dwUrlPathLength); 
	urlComp.dwExtraInfoLength = 1024*32;
	urlComp.lpszExtraInfo = (wchar_t*) xmlMalloc((size_t) urlComp.dwExtraInfoLength); /* ������� ������ - � POST */
	iconv_string("utf-16le", "utf-8", params->uri, params->uri + strlen(params->uri), (char**) &wszRequestUrl, NULL);
	if (!WinHttpCrackUrl(wszRequestUrl, 0, 0, &urlComp))
	{
		params->error = xefCreateCommonErrorMessage(BAD_CAST "the specified URI \"%s\" is incorrect", params->uri);
		goto done;
	}

	/* instantiate a connection (not really opening it yet) */
	hConnect = WinHttpConnect(hSession, urlComp.lpszHostName, urlComp.nPort, 0);
	if (!hConnect)
	{
		params->error = xefCreateTransportErrorMessage(BAD_CAST "cannot connect to the specified server");
		goto done;
	}

	/* instantiate a request */
	options = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
	/* wszRequestUrl �� ������, ��� ��� ��� ���������� */
	wcscpy(wszRequestUrl, urlComp.lpszUrlPath);
	wcscat(wszRequestUrl, urlComp.lpszExtraInfo);
	hRequest = WinHttpOpenRequest(hConnect, params->extra_query? L"POST": L"GET", wszRequestUrl,
		NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,	options);
	if (!hRequest)
	{
		params->error = xefCreateTransportErrorMessage(BAD_CAST "cannot open WinHTTP request");
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
		szProxy = xplFormatMessage("%s:%d", cfgProxyServer, cfgProxyPort?cfgProxyPort:80);
		iconv_string("utf-16le", "utf-8", (char*) szProxy, (char*) (szProxy + strlen(szProxy)), (char**) &proxyInfo.lpszProxy, NULL);
		if (!WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo)))
		{
			params->error = xefCreateTransportErrorMessage(BAD_CAST "cannot set up proxy");
			goto done;
		}
		if (cfgProxyUser)
		{
			iconv_string("utf-16le", "utf-8", (char*) cfgProxyUser, (char*) (cfgProxyUser + xmlStrlen(cfgProxyUser)), (char**) &wszProxyUser, &iconv_len);
			if (!WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY_USERNAME, (LPVOID) wszProxyUser, (DWORD) iconv_len))
			{
				params->error = xefCreateTransportErrorMessage(BAD_CAST "cannot set up proxy user");
				goto done;
			}
			if (cfgProxyPassword)
			{
				iconv_string("utf-16le", "utf-8", (char*) cfgProxyPassword, (char*)(cfgProxyPassword + xmlStrlen(cfgProxyPassword)), (char**) &wszProxyPassword, &iconv_len);
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

	/* Retry for several times if fails. */
	while (!bResponseSucceeded && iRetryTimes++ < INT_RETRYTIMES)
	{
		if (!WinHttpSendRequest(hRequest, wsz_extra_headers, -1, params->extra_query, extra_query_len, extra_query_len, (DWORD_PTR) NULL))
		{
			if (iRetryTimes == INT_RETRYTIMES)
			{
				params->error = xefCreateTransportErrorMessage(BAD_CAST "cannot send request");
				goto done;
			} else
				continue; /* next try */
		}
		if (!WinHttpReceiveResponse(hRequest, NULL))
		{
			if (iRetryTimes == INT_RETRYTIMES)
			{
				params->error = xefCreateTransportErrorMessage(BAD_CAST "cannot receive server response");
				goto done;
			} else
				continue; /* next try */
		}
#if 0
		/* ToDo: ������� ��������� */
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
		/* ��������� ������� */
		params->encoding = NULL;
		/* ������� ��� ������� */
		/* ToDo: ��������� ��� */
		dwSize = 0;
		bOpResult = WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE, WINHTTP_HEADER_NAME_BY_INDEX, NULL, &dwSize, WINHTTP_NO_HEADER_INDEX);
		if (bOpResult || (!bOpResult && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)))
		{
			wszHeader = (wchar_t*) xmlMalloc((size_t) dwSize*sizeof(wchar_t));
			if (wszHeader != NULL)
			{
				memset(wszHeader, 0, (size_t) dwSize* sizeof(wchar_t));
				if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE, WINHTTP_HEADER_NAME_BY_INDEX, wszHeader, &dwSize, WINHTTP_NO_HEADER_INDEX))
				{
					swscanf_s(wszHeader, L"%d", &status_code, dwSize);
					params->status_code = status_code;
				}
				xmlFree(wszHeader);
			}
		} /* if status queried */

		buf = rbCreateBufParams(16384, RESZ_BUF_GROW_DOUBLE, 2);
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
			if (rbEnsureBufFreeSize(buf, (size_t) dwSize) != RESZ_BUF_RESULT_OK)
			{
				params->error = xefCreateCommonErrorMessage(BAD_CAST "insufficient memory");
				goto done;
			}
			params->document = rbGetBufPosition(buf);
			if (!WinHttpReadData(hRequest, params->document, dwSize, &dwRead))
			{
				if (iRetryTimes == INT_RETRYTIMES)
				{
					params->error = xefCreateTransportErrorMessage(BAD_CAST "cannot read server response (!?)");
					goto done;
				} else
					continue; /* next try */
			}
			rbAdvanceBufPosition(buf, (size_t) dwRead);
			dwTotal += dwRead;
		} while (dwSize > 0); /* reading cycle */
		bResponseSucceeded = true;
	} // while
	rbAddDataToBuf(buf, &zero, sizeof(zero));
	params->document = (xmlChar*) rbDetachBufContent(buf);
	params->document_size = (size_t) dwTotal;
	params->error = NULL;
	/* ToDo: ��������� */
	params->real_uri = NULL;
done:
	if (hSession) WinHttpCloseHandle(hSession);
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);

	if (wszRequestUrl) xmlFree(wszRequestUrl);
	if (urlComp.lpszHostName) xmlFree(urlComp.lpszHostName);
	if (urlComp.lpszUrlPath) xmlFree(urlComp.lpszUrlPath);
	if (urlComp.lpszExtraInfo) xmlFree(urlComp.lpszExtraInfo);
	if (szProxy) xmlFree(szProxy);
	if (proxyInfo.lpszProxy) xmlFree(proxyInfo.lpszProxy);
	if (wszProxyUser) xmlFree(wszProxyUser);
	if (wszProxyPassword) xmlFree(wszProxyPassword);

	if (buf) rbFreeBuf(buf);

	return (params->error)? false: true;
}