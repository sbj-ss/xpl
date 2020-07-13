#include <stdlib.h>
#include <string.h>
#include <Configuration.h>
#include <libxpl/abstraction/xef.h>
#include <libxpl/xplstart.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#ifdef _USE_TCMALLOC
#include "tcmalloc.h"
#endif

#ifdef _USE_TCMALLOC
static char* XMLCALL tc_strdup(const char *src)
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

static bool startMem(const xplStartParamsPtr params, int argc, const char **argv, xmlChar **error)
{
	int ret = 0;

	UNUSED_PARAM(argc);
	UNUSED_PARAM(argv);

	if (params->debug_allocation && params->use_tcmalloc)
	{
		if (error)
			*error = xmlStrdup(BAD_CAST ".debug_allocation and .use_tcmalloc can't be set simultaneously");
		return false;
	}
	if (params->use_tcmalloc)
	{
#ifdef _USE_TCMALLOC
		ret = xmlMemSetup(tc_free, tc_malloc, tc_realloc, tc_strdup);
#else
		if (error)
			*error = xmlStrdup(BAD_CAST "tcmalloc support not compiled in");
		return false;
#endif
	}
	if (params->debug_allocation)
		ret = xmlMemSetup(xmlMemFree, xmlMemMalloc, xmlMemRealloc, xmlMemoryStrdup);
	if (ret)
	{
		if (error)
			*error = xmlStrdup(BAD_CAST "xmlMemSetup() failed");
		return false;
	}
	return true;
}

static void stopMem(void)
{
	xmlMemSetup(free, malloc, realloc, strdup);
}

static bool startXpr(const xplStartParamsPtr params, int argc, const char **argv, xmlChar **error)
{
	UNUSED_PARAM(argc);
	UNUSED_PARAM(argv);

	if (!xprParseCommandLine())
	{
		if (error)
			*error = xmlStrdup(BAD_CAST "xprParseCommandLine() failed");
		return false;
	}
	if (!xprStartup(params->xpr_start_flags))
	{
		if (error)
			*error = xmlStrdup(BAD_CAST "xplStartup() failed");
		return false;
	}
	return true;
}

static void stopXpr(void)
{
	xprShutdown(xprGetActiveSubsystems());
}

static bool startXml(const xplStartParamsPtr params, int argc, const char **argv, xmlChar **error)
{
	UNUSED_PARAM(params);
	UNUSED_PARAM(argc);
	UNUSED_PARAM(argv);

	LIBXML_TEST_VERSION // TODO replace with nondestructive behavior. setjmp?..
	xmlInitParser(); /* no return codes here */
	return true;
}

static void stopXml(void)
{
	xmlCleanupParser();
}

static bool startXef(const xplStartParamsPtr params, int argc, const char **argv, xmlChar **error)
{
	xefStartupParams xef_params;
	xmlChar *xef_error_text;

	UNUSED_PARAM(params);
	UNUSED_PARAM(argc);
	UNUSED_PARAM(argv);

	xplDisplayMessage(xplMsgInfo, BAD_CAST "Starting external libraries...");
	if (!xefStartup(&xef_params))
	{
		if (xef_params.error)
		{
			if (error)
				*error = xefGetErrorText(xef_params.error);
			xefFreeErrorMessage(xef_params.error);
		} else if (error)
			*error = xmlStrdup(BAD_CAST "external libraries startup failed (unknown error)");
		return false;
	}
	return true;
}

static void stopXef(void)
{
	xefShutdown();
}

static bool startXpl(const xplStartParamsPtr params, int argc, const char **argv, xmlChar **error)
{
	xmlChar* conf_path;
	char *fn_pos;
	xplError err_code;

	if (xmlStrchr(params->config_file_name, XPR_PATH_DELIM))
		conf_path = xmlStrdup(params->config_file_name);
	else {
		/* argv[0] can be just "xplweb" on windows :\ */
		fn_pos = strrchr(argv[0], XPR_PATH_DELIM);
		if (fn_pos)
		{
			conf_path = (xmlChar*) xmlMalloc(strlen(argv[0]) + xmlStrlen(params->config_file_name) + 1);
			strncpy((char*) conf_path, argv[0], fn_pos - argv[0] + 1);
			strcat((char*) conf_path, params->config_file_name);
		} else
			conf_path = xmlStrdup(BAD_CAST params->config_file_name);
	}

	err_code = xplInitParser(conf_path);
	xmlFree(conf_path);
	if (err_code != XPL_ERR_NO_ERROR)
	{
		if (error)
			*error = xplFormatMessage(BAD_CAST "error starting interpreter: \"%s\"", xplErrorToString(err_code));
		return false;
	}
	return true;
}

static void stopXpl(void)
{
	xplDoneParser();
}

typedef struct _StartStopStep
{
	bool (*start_fn)(const xplStartParamsPtr params, int argc, const char **argv, xmlChar **error);
	void (*stop_fn)(void);
} StartStopStep, *StartStopStepPtr;

static StartStopStep start_stop_steps[] =
{
	{ startMem, stopMem },
	{ startXpr, stopXpr },
	{ startXml, stopXml },
	{ startXef, stopXef },
	{ startXpl, stopXpl }
};
#define START_STOP_STEP_COUNT (sizeof(start_stop_steps) / sizeof(start_stop_steps[0]))

bool xplStartEngine(const xplStartParamsPtr params, int argc, const char **argv, xmlChar **error)
{
	int i, j;

	for (i = 0; i < START_STOP_STEP_COUNT; i++)
		if (!start_stop_steps[i].start_fn(params, argc, argv, error))
		{
			for (j = i - 1; j >= 0; j--)
				start_stop_steps[j].stop_fn();
			return false;
		}
	return true;
}

void xplShutdownEngine()
{
	int i;

	for (i = START_STOP_STEP_COUNT; i >= 0; i--)
		start_stop_steps[i].stop_fn();
}
