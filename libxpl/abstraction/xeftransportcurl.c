#include <curl/curl.h>
#include <libxpl/abstraction/xef.h>
#include <libxpl/xplbuffer.h>
#include <libxpl/xploptions.h>

#define USER_AGENT_STRING "User-Agent: Mozilla/5.0 (X11; Linux x86_64) " \
    "AppleWebKit/537.36 (KHTML, like Gecko) Ubuntu Chromium/47.0.2526.73 " \
    "Chrome/47.0.2526.73 Safari/537.36"

#ifdef _WIN32
#include <processenv.h>
#define CA_BUNDLE_FILE_NAME "cacert.pem"

static char *ca_bundle_path;

static char* _getCaBundleLocation()
{
	char *buffer, *file_part;
	DWORD size;

	if (!(size = SearchPathA(NULL, CA_BUNDLE_FILE_NAME, NULL, 0, NULL, NULL)))
		return NULL;
	if (!(buffer = XPL_MALLOC(size)))
		return NULL;
	SearchPathA(NULL, CA_BUNDLE_FILE_NAME, NULL, size, buffer, &file_part);
	return buffer;
}

static bool _initSSL(xefStartupParamsPtr params)
{
	if (!(ca_bundle_path = _getCaBundleLocation()))
	{
		params->error = BAD_CAST XPL_STRDUP("Can't locate "CA_BUNDLE_FILE_NAME);
		return false;
	}
	return true;
}

static bool _setCurlSSL(CURL *curl)
{
	return (curl_easy_setopt(curl, CURLOPT_CAINFO, ca_bundle_path) == CURLE_OK);
}

static void _cleanupSSL(void)
{
	if (ca_bundle_path)
	{
		XPL_FREE(ca_bundle_path);
		ca_bundle_path = NULL;
	}
}
#else
static bool _initSSL(xefStartupParamsPtr params)
{
	UNUSED_PARAM(params);
	return true;
}

static bool _setCurlSSL(CURL *curl)
{
	/* we rely on builtins in linux */
	UNUSED_PARAM(curl);
	return true;
}

static void _cleanupSSL(void)
{
}
#endif

bool xefStartupTransport(xefStartupParamsPtr params)
{
	CURLcode res;

	if ((res = curl_global_init(CURL_GLOBAL_ALL)) != CURLE_OK)
	{
		params->error = BAD_CAST XPL_STRDUP(curl_easy_strerror(res));
		return false;
	}
	if (!_initSSL(params))
		return false;
	params->error = NULL;
	return true;
}

static size_t _writeCallback(char *ptr, size_t size, size_t nItems, void *userData)
{
    rbBufPtr buf = (rbBufPtr) userData;
    size_t total_size = size * nItems;

    if (rbAddDataToBuf(buf, ptr, total_size) == RB_RESULT_OK)
        return total_size;
    else
        return 0;
}

typedef struct _uploadData
{
	xmlChar *data;
	size_t pos;
	size_t size;
} uploadData, *uploadDataPtr;

static size_t _readCallback(char *ptr, size_t size, size_t nItems, void *userData)
{
	uploadDataPtr data = (uploadDataPtr) userData;
	size_t left, avail;

	avail = size * nItems;
	left = data->size - data->pos;
	if (avail < left)
		left = avail;
	memcpy(ptr, data->data, left);
	data->pos += left;
	return left;
}

bool xefFetchDocument(xefFetchDocumentParamsPtr params)
{
	CURL *curl;
	rbBufPtr buf;
	uploadData post_data;
	CURLcode res;
	long status;
	char *content_type, *encoding;
	struct curl_slist* curl_headers = NULL;

	if (!(curl = curl_easy_init()))
	{
		params->error = BAD_CAST XPL_STRDUP("curl_easy_init() failed");
		goto done;
	}
	if (!_setCurlSSL(curl))
	{
		params->error = BAD_CAST XPL_STRDUP("setting SSL options failed");
		goto done;
	}
	if (!(buf = rbCreateBufParams(0x10000, RB_GROW_DOUBLE, 0)))
	{
		params->error = BAD_CAST XPL_STRDUP("rbCreateBufParams() failed");
		goto done;
	}
	curl_headers = curl_slist_append(curl_headers, USER_AGENT_STRING);
	curl_easy_setopt(curl, CURLOPT_URL, (char*) params->uri);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _writeCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, buf);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);
#if 0
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#endif
	if (cfgProxyServer)
	{
		curl_easy_setopt(curl, CURLOPT_PROXY, (char*) cfgProxyServer);
		curl_easy_setopt(curl, CURLOPT_PROXYPORT, cfgProxyPort);
		if (cfgProxyUser)
		{
			curl_easy_setopt(curl, CURLOPT_PROXYUSERNAME, (char*) cfgProxyUser);
			curl_easy_setopt(curl, CURLOPT_PROXYPASSWORD, (char*) cfgProxyPassword);
		}
	}
	if (params->extra_query)
	{
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		post_data.data = params->extra_query;
		post_data.size = xmlStrlen(params->extra_query);
		post_data.pos = 0;
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post_data.size);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, _readCallback);
		curl_easy_setopt(curl, CURLOPT_READDATA, &post_data);
	}

	if ((res = curl_easy_perform(curl)) != CURLE_OK)
	{
		params->error = BAD_CAST XPL_STRDUP(curl_easy_strerror(res));
		goto done;
	}
	if (rbAddDataToBuf(buf, "", 1) != RB_RESULT_OK)
	{
		params->error = BAD_CAST XPL_STRDUP("Out of memory");
		goto done;
	}
	params->document_size = rbGetBufContentSize(buf);
	params->document = rbDetachBufContent(buf);
	curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, (char**) &params->real_uri);
	params->real_uri = BAD_CAST XPL_STRDUP((char*) params->real_uri);
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
	params->status_code = status;
	curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type);
	if ((encoding = strstr(content_type, "charset=")))
		params->encoding = BAD_CAST XPL_STRDUP(encoding + 8);
	else
		params->encoding = NULL;
done:
	if (curl)
		curl_easy_cleanup(curl);
	if (buf)
		rbFreeBuf(buf);
	if (curl_headers)
		curl_slist_free_all(curl_headers);
	return !params->error;
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

void xefShutdownTransport(void)
{
	_cleanupSSL();
}
