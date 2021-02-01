#include <stdio.h>
#include <libxpl/abstraction/xef.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>

void xplCmdGetVersionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

#if defined(__GNUC__) & !defined(__MINGW32__)
	#include <gnu/libc-version.h>
#else
	#include <iconv.h>
#endif
#ifdef _USE_LIBIDN
	#include <stringprep.h>
#endif
#include <oniguruma.h>
#ifdef _USE_LZMA
	#include <lzma.h>
#endif
#ifdef _USE_ZLIB
	#include <zlib.h>
#endif
#ifdef _XEF_HTML_CLEANER_TIDY
	#include <tidy/tidy.h>
#endif

typedef enum _xplCmdGetVersionPart
{
	XCGV_PART_MAJOR,
	XCGV_PART_MINOR,
	XCGV_PART_FULL,
	XCGV_PART_LIBS
} xplCmdGetVersionPart;

static xplCmdParamDictValue part_dict[] =
{
	{ BAD_CAST "minor", XCGV_PART_MINOR },
	{ BAD_CAST "major", XCGV_PART_MAJOR },
	{ BAD_CAST "full", XCGV_PART_FULL },
	{ BAD_CAST "libs", XCGV_PART_LIBS },
	{ NULL, 0 }
};

typedef struct _xplCmdGetVersionParams
{
	xplCmdGetVersionPart part;
	xplQName tagname;
	bool repeat;
} xplCmdGetVersionParams, *xplCmdGetVersionParamsPtr;

static const xplCmdGetVersionParams params_stencil =
{
	.part = XCGV_PART_FULL,
	.tagname = { NULL, BAD_CAST "library" },
	.repeat = true
};

xplCommand xplGetVersionCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdGetVersionEpilogue,
	.flags = XPL_CMD_FLAG_PARAMS_FOR_EPILOGUE,
	.params_stencil = &params_stencil,
	.stencil_size = sizeof(xplCmdGetVersionParams),
	.parameters = {
		{
			.name = BAD_CAST "part",
			.type = XPL_CMD_PARAM_TYPE_DICT,
			.extra.dict_values = part_dict,
			.value_stencil = &params_stencil.part
		}, {
			.name = BAD_CAST "tagname",
			.type = XPL_CMD_PARAM_TYPE_QNAME,
			.value_stencil = &params_stencil.tagname
		}, {
			.name = BAD_CAST "repeat",
			.type = XPL_CMD_PARAM_TYPE_BOOL,
			.value_stencil = &params_stencil.repeat
		}, {
			.name = NULL
		}
	}
};

#define NAME_A (BAD_CAST "name")
#define COMPILED_A (BAD_CAST "compiled")
#define RUNNING_A (BAD_CAST "running")
#define NOT_CHECKABLE (BAD_CAST "not checkable")

typedef void (*versionGetter)(xmlNodePtr carrier);

static void _getLibiconvVersion(xmlNodePtr carrier)
{
	char buf[16];

	xmlNewProp(carrier, NAME_A, BAD_CAST "libiconv");
#if defined(__GNUC__) & !defined(__MINGW32__)
	snprintf(buf, sizeof(buf), "%u.%u", __GLIBC__, __GLIBC_MINOR__);
	xmlNewProp(carrier, COMPILED_A, BAD_CAST buf);
	xmlNewProp(carrier, RUNNING_A, BAD_CAST gnu_get_libc_version());
#else
	snprintf(buf, sizeof(buf), "%d.%d", _LIBICONV_VERSION >> 8, _LIBICONV_VERSION & 0xFF);
	xmlNewProp(carrier, COMPILED_A, BAD_CAST buf);
	xmlNewProp(carrier, RUNNING_A, NOT_CHECKABLE);
#endif
}

#ifdef _USE_LIBIDN
static void _getLibidnVersion(xmlNodePtr carrier)
{
	xmlNewProp(carrier, NAME_A, BAD_CAST "libidn");
	xmlNewProp(carrier, COMPILED_A, BAD_CAST STRINGPREP_VERSION);
	xmlNewProp(carrier, RUNNING_A, BAD_CAST stringprep_check_version(NULL));
}
#endif

#ifdef _USE_LZMA
static void _getLiblzmaVersion(xmlNodePtr carrier)
{
	xmlNewProp(carrier, NAME_A, BAD_CAST "lzma");
	xmlNewProp(carrier, COMPILED_A, BAD_CAST LZMA_VERSION_STRING);
	xmlNewProp(carrier, RUNNING_A, BAD_CAST lzma_version_string());
}
#endif

static void _getOnigurumaVersion(xmlNodePtr carrier)
{
	char buf[16];

	xmlNewProp(carrier, NAME_A, BAD_CAST "oniguruma");
	snprintf(buf, sizeof(buf), "%d.%d.%d", ONIGURUMA_VERSION_MAJOR, ONIGURUMA_VERSION_MINOR, ONIGURUMA_VERSION_TEENY);
	xmlNewProp(carrier, COMPILED_A, BAD_CAST buf);
	xmlNewProp(carrier, RUNNING_A, BAD_CAST onig_version());
}

static void _getLibxml2Version(xmlNodePtr carrier)
{
	xmlNewProp(carrier, NAME_A, BAD_CAST "libxml2");
	xmlNewProp(carrier, COMPILED_A, BAD_CAST LIBXML_VERSION_STRING""LIBXML_VERSION_EXTRA);
	xmlNewProp(carrier, RUNNING_A, BAD_CAST xmlParserVersion);
}

#ifdef _USE_ZLIB
static void _getZlibVersion(xmlNodePtr carrier)
{
	xmlNewProp(carrier, NAME_A, BAD_CAST "zlib");
	xmlNewProp(carrier, COMPILED_A, BAD_CAST ZLIB_VERSION);
	xmlNewProp(carrier, RUNNING_A, BAD_CAST zlibVersion());
}
#endif

#ifdef _XEF_HTML_CLEANER_TIDY
static void _getTidyVersion(xmlNodePtr carrier)
{
	/* Tidy doesn't export its version */
	xmlNewProp(carrier, NAME_A, BAD_CAST "htmltidy");
	xmlNewProp(carrier, COMPILED_A, BAD_CAST tidyLibraryVersion());
	xmlNewProp(carrier, RUNNING_A, NOT_CHECKABLE);
}
#endif

/* ToDo: transport */

static versionGetter version_getters[] =
{
	_getLibiconvVersion,
#ifdef _USE_LIBIDN
	_getLibidnVersion,
#endif
#ifdef _USE_LZMA
	_getLiblzmaVersion,
#endif
	_getOnigurumaVersion,
	_getLibxml2Version,
#ifdef _USE_ZLIB
	_getZlibVersion,
#endif
#ifdef _XEF_HTML_CLEANER_TIDY
	_getTidyVersion,
#endif
};

static xmlNodePtr _listLibraryVersions(xmlDocPtr doc, xplQName tagname)
{
	xmlNodePtr ret = NULL, tail = NULL, cur;
	static const ssize_t getter_count = sizeof(version_getters) / sizeof(version_getters[0]);
	ssize_t i;

	for (i = 0; i < getter_count; i++)
	{
		cur = xmlNewDocNode(doc, tagname.ns, tagname.ncname, NULL);
		version_getters[i](cur);
		if (tail)
		{
			tail->next = cur;
			cur->prev = tail;
			tail = cur;
		} else
			ret = tail = cur;
	}
	return ret;
}

static xmlNodePtr _getXplVersion(xmlDocPtr doc, xplCmdGetVersionPart part)
{
	xmlChar *ver;
	xmlNodePtr ret;

	switch (part)
	{
		case XCGV_PART_MAJOR:
			ver = (xmlChar*) XPL_MALLOC(12);
			sprintf((char*) ver, "%d", XPL_VERSION_MAJOR);
			break;
		case XCGV_PART_MINOR:
			ver = (xmlChar*) XPL_MALLOC(12);
			sprintf((char*) ver, "%d", XPL_VERSION_MINOR);
			break;
		case XCGV_PART_FULL:
			ver = BAD_CAST XPL_STRDUP((char*) XPL_VERSION_FULL);
			break;
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
			ver = BAD_CAST XPL_STRDUP("unknown part requested");
	}
	ret = xmlNewDocText(doc, NULL);
	ret->content = ver;
	return ret;
}

void xplCmdGetVersionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	xplCmdGetVersionParamsPtr params = (xplCmdGetVersionParamsPtr) commandInfo->params;

	if (params->part == XCGV_PART_LIBS)
		ASSIGN_RESULT(_listLibraryVersions(commandInfo->element->doc, params->tagname), params->repeat, true);
	else
		ASSIGN_RESULT(_getXplVersion(commandInfo->element->doc, params->part), false, true);
}
