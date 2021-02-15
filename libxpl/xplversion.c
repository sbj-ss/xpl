#include <errno.h>
#include <stdio.h>
#include <libxpl/xplbuffer.h>
#include <libxpl/xplversion.h>

#ifdef _XEF_TRANSPORT_CURL
	#include <curl/curl.h>
#endif
#if defined(__GNUC__) & !defined(__MINGW32__)
	#include <gnu/libc-version.h>
#else
	#include <iconv.h>
#endif
#ifdef _USE_LIBIDN
	#include <stringprep.h>
#endif
#ifdef _USE_LIBLZMA
	#include <lzma.h>
#endif
#include <oniguruma.h>
#include <openssl/crypto.h>
#include <openssl/opensslv.h>
#ifdef _XEF_HTML_CLEANER_TIDY
	#include <tidy/tidy.h>
#endif
#include <yajl/yajl_version.h>
#ifdef _USE_ZLIB
	#include <zlib.h>
#endif

#define NOT_CHECKABLE (BAD_CAST "not checkable")
#define NOT_COMPILED_IN (BAD_CAST "not compiled in")

typedef void (*versionGetter)(const xmlChar **str, bool running);

static void _getLibCurlVersion(const xmlChar **str, bool running)
{
#ifndef _XEF_TRANSPORT_CURL
	UNUSED_PARAM(running);
	*str = NOT_COMPILED_IN;
#else
	if (running)
		*str = BAD_CAST curl_version();
	else
		*str = BAD_CAST LIBCURL_VERSION;
#endif
}

static void _getLibIconvVersion(const xmlChar **str, bool running)
{
	static char buf[16];

#if defined(__GNUC__) & !defined(__MINGW32__)
	if (running)
		*str = BAD_CAST gnu_get_libc_version();
	else {
		snprintf(buf, sizeof(buf), "%u.%u", __GLIBC__, __GLIBC_MINOR__);
		*str = BAD_CAST buf;
	}
#else
	if (running)
		*str = NOT_CHECKABLE;
	else {
		snprintf(buf, sizeof(buf), "%d.%d", _LIBICONV_VERSION >> 8, _LIBICONV_VERSION & 0xFF);
		*str = BAD_CAST buf;
	}
#endif
}

static void _getLibIdnVersion(const xmlChar **str, bool running)
{
#ifndef _USE_LIBIDN
	UNUSED_PARAM(running);
	*str = NOT_COMPILED_IN;
#else
	if (running)
		*str = BAD_CAST stringprep_check_version(NULL);
	else
		*str = BAD_CAST STRINGPREP_VERSION;
#endif
}

static void _getLibLzmaVersion(const xmlChar **str, bool running)
{
#ifndef _USE_LIBLZMA
	UNUSED_PARAM(running);
	*str = NOT_COMPILED_IN;
#else
	if (running)
		*str = BAD_CAST lzma_version_string();
	else
		*str = BAD_CAST LZMA_VERSION_STRING;
#endif
}

static void _getUnixOdbcVersion(const xmlChar **str, bool running)
{
#ifndef _XEF_DB_ODBC
	UNUSED_PARAM(running);
	*str = NOT_COMPILED_IN;
#else
	static char buf[48];
	FILE *fp;
	/* For some obscure reason unixODBC does not provide a way to obtain its version via headers/functions etc.
	 * The only possible method left is to exec `odbcinst --version'. */
	if (running)
	{
		if (!(fp = popen("odbcinst --version", "r")))
			snprintf(buf, sizeof(buf), "failed to execute odbcinst (errno=%d)", errno);
		else {
			fgets(buf, sizeof(buf), fp);
			pclose(fp);
		}
		buf[sizeof(buf) - 1] = 0;
		if (buf[0])
			buf[strlen(buf) - 1] = 0; // remove \n
		*str = BAD_CAST buf;
	} else
		*str = NOT_CHECKABLE;
#ifndef _WIN32
#else
	UNUSED_PARAM(running);
	*str = BAD_CAST "Windows native";
#endif
#endif
}

static void _getOnigurumaVersion(const xmlChar **str, bool running)
{
	static char buf[16];

	if (running)
		*str = BAD_CAST onig_version();
	else {
		snprintf(buf, sizeof(buf), "%d.%d.%d", ONIGURUMA_VERSION_MAJOR, ONIGURUMA_VERSION_MINOR, ONIGURUMA_VERSION_TEENY);
		*str = BAD_CAST buf;
	}
}

static void _getOpenSslVersion(const xmlChar **str, bool running)
{
	if (running)
		*str = BAD_CAST OpenSSL_version(OPENSSL_VERSION);
	else
		*str = BAD_CAST OPENSSL_VERSION_TEXT;
}

static void _getTidyVersion(const xmlChar **str, bool running)
{
#ifndef _XEF_HTML_CLEANER_TIDY
	UNUSED_PARAM(running);
	*str = NOT_COMPILED_IN;
#else
	if (running)
		*str = NOT_CHECKABLE;
	else
		*str = BAD_CAST tidyLibraryVersion();
#endif
}

static void _getLibxml2Version(const xmlChar **str, bool running)
{
	static char buf[] = LIBXML_VERSION_STRING""LIBXML_VERSION_EXTRA;

	if (running)
		*str = BAD_CAST xmlParserVersion;
	else
		*str = BAD_CAST buf;
}

static void _getLibYajlVersion(const xmlChar **str, bool running)
{
	static char compiled_v[32], running_v[32];
	int ver;

	if (running)
	{
		ver = yajl_version();
		snprintf(running_v, sizeof(running_v), "%d.%d.%d", ver / 10000, (ver % 10000) / 100, ver % 100);
		running_v[sizeof(running_v) - 1] = 0;
		*str = BAD_CAST running_v;
	} else {
		snprintf(compiled_v, sizeof(compiled_v), "%d.%d.%d", YAJL_MAJOR, YAJL_MINOR, YAJL_MICRO);
		compiled_v[sizeof(compiled_v) - 1] = 0;
		*str = BAD_CAST compiled_v;
	}
}


static void _getZlibVersion(const xmlChar **str, bool running)
{
#ifndef _USE_ZLIB
	UNUSED_PARAM(running);
	*str = NOT_COMPILED_IN;
#else
	if (running)
		*str = BAD_CAST zlibVersion();
	else
		*str = BAD_CAST ZLIB_VERSION;
#endif
}

typedef struct _versionElement
{
	xmlChar *name;
	versionGetter getter;
	ptrdiff_t offset;
} versionElement, *versionElementPtr;

static const xplLibraryVersions * const version_stencil = NULL;

#define VERSION_OFFSET(part) (uintptr_t) &version_stencil->part - (uintptr_t) version_stencil

static const versionElement version_elements[] =
{
	{ BAD_CAST "libcurl", _getLibCurlVersion, VERSION_OFFSET(curl_version) },
	{ BAD_CAST "libiconv", _getLibIconvVersion, VERSION_OFFSET(iconv_version) },
	{ BAD_CAST "libidn", _getLibIdnVersion, VERSION_OFFSET(idn_version) },
	{ BAD_CAST "liblzma", _getLibLzmaVersion, VERSION_OFFSET(lzma_version) },
	{ BAD_CAST "unixodbc", _getUnixOdbcVersion, VERSION_OFFSET(odbc_version), },
	{ BAD_CAST "oniguruma", _getOnigurumaVersion, VERSION_OFFSET(onig_version) },
	{ BAD_CAST "openssl", _getOpenSslVersion, VERSION_OFFSET(ssl_version) },
	{ BAD_CAST "htmltidy", _getTidyVersion, VERSION_OFFSET(tidy_version) },
	{ BAD_CAST "libxml2", _getLibxml2Version, VERSION_OFFSET(xml2_version) },
	{ BAD_CAST "libyajl", _getLibYajlVersion, VERSION_OFFSET(yajl_version) },
	{ BAD_CAST "zlib", _getZlibVersion, VERSION_OFFSET(z_version) }
};

#define VERSION_ELEMENT_COUNT (sizeof(version_elements) / sizeof(version_elements[0]))
#define OFFSET_STR(version, offset) ((const xmlChar**) ((uintptr_t) (version) + (offset)))

int xplVersion()
{
	return XPL_VERSION;
}

xmlChar* xplVersionString()
{
	return XPL_VERSION_FULL;
}

static xplLibraryVersions compiled_versions;
static xplLibraryVersions running_versions;
bool versions_initialized = false;

static void _initLibraryVersions()
{
	size_t i;

	for (i = 0; i < VERSION_ELEMENT_COUNT; i++)
	{
		version_elements[i].getter(OFFSET_STR(&compiled_versions, version_elements[i].offset), false);
		version_elements[i].getter(OFFSET_STR(&running_versions, version_elements[i].offset), true);
	}
}

xplLibraryVersionsPtr xplGetCompiledLibraryVersions()
{
	if (!versions_initialized)
	{
		_initLibraryVersions();
		versions_initialized = true;
	}
	return &compiled_versions;
}

xplLibraryVersionsPtr xplGetRunningLibraryVersions()
{
	if (!versions_initialized)
	{
		_initLibraryVersions();
		versions_initialized = true;
	}
	return &running_versions;
}

xmlNodePtr xplLibraryVersionsToNodeList(xmlDocPtr doc, xplQName tagname, xplLibraryVersionsPtr compiled, xplLibraryVersionsPtr running)
{
	xmlNodePtr ret = NULL, tail = NULL, cur;
	size_t i;

	for (i = 0; i < VERSION_ELEMENT_COUNT; i++)
	{
		if (!(cur = xmlNewDocNode(doc, tagname.ns, tagname.ncname, NULL)))
			goto oom;
		if (tail)
		{
			tail->next = cur;
			cur->prev = tail;
			tail = cur;
		} else
			ret = tail = cur;
		if (!xmlNewProp(cur, BAD_CAST "name", version_elements[i].name))
			goto oom;
		if (!xmlNewProp(cur, BAD_CAST "compiled", *OFFSET_STR(compiled, version_elements[i].offset)))
			goto oom;
		if (!xmlNewProp(cur, BAD_CAST "running", *OFFSET_STR(running, version_elements[i].offset)))
			goto oom;
	}
	return ret;
oom:
	if (ret)
		xmlFreeNode(ret);
	return NULL;
}

xmlChar* xplLibraryVersionsToString(xplLibraryVersionsPtr compiled, xplLibraryVersionsPtr running)
{
	size_t i;
	rbBufPtr buf;
	rbOpResult result = 0;
	xmlChar *ret;

	if (!(buf = rbCreateBufParams(512, RB_GROW_INCREMENT, 128)))
		return NULL;
	for (i = 0; i < VERSION_ELEMENT_COUNT; i++)
	{
		result |= rbAddStringToBuf(buf, version_elements[i].name);
		result |= rbAddStringToBuf(buf, BAD_CAST ": compiled=");
		result |= rbAddStringToBuf(buf, *OFFSET_STR(compiled, version_elements[i].offset));
		result |= rbAddStringToBuf(buf, BAD_CAST ", running=");
		result |= rbAddStringToBuf(buf, *OFFSET_STR(running, version_elements[i].offset));
		result |= rbAddStringToBuf(buf, BAD_CAST "\n");
		if (result != RB_RESULT_OK)
			goto error;
	}
	if (rbAddDataToBuf(buf, "", 1) != RB_RESULT_OK)
		goto error;
	ret = BAD_CAST rbDetachBufContent(buf);
	rbFreeBuf(buf);
	return ret;
error:
	rbFreeBuf(buf);
	return NULL;
}

bool xplInitLibraryVersions()
{
	// library names are lazy-loaded on first call
	return true;
}

void xplCleanupLibraryVersions()
{
	// nothing to do here at the moment
}
