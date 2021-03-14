#include <stdlib.h>
#include <string.h>
#include <Configuration.h>
#include <libxpl/abstraction/xpr.h>
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

bool xplInitMemory(bool debugAllocation, bool useTcmalloc)
{
	if (debugAllocation && useTcmalloc)
		return false;
	if (useTcmalloc)
	{
#ifdef _USE_TCMALLOC
		ret = xmlMemSetup(tc_free, tc_malloc, tc_realloc, tc_strdup);
#else
		fprintf(stderr, "tcmalloc support not compiled in");
		return false;
#endif
	}
	if (debugAllocation)
		if (xmlMemSetup(xmlMemFree, xmlMemMalloc, xmlMemRealloc, xmlMemoryStrdup))
			return false;
	return true;
}

void xplCleanupMemory(void)
{
	xmlMemSetup(free, malloc, realloc, strdup);
}

static bool _startXpr(const xplStartParamsPtr params, int argc, const char **argv, xmlChar **error)
{
	UNUSED_PARAM(argc);
	UNUSED_PARAM(argv);

	if (!xprParseCommandLine())
	{
		if (error)
			*error = BAD_CAST XPL_STRDUP("xprParseCommandLine() failed");
		return false;
	}
	if (!xprStartup(params->xpr_start_flags))
	{
		if (error)
			*error = BAD_CAST XPL_STRDUP("xplStartup() failed");
		return false;
	}
	return true;
}

static void _stopXpr(void)
{
	xprShutdown(xprGetActiveSubsystems());
}

static bool _startXml(const xplStartParamsPtr params, int argc, const char **argv, xmlChar **error)
{
	UNUSED_PARAM(params);
	UNUSED_PARAM(argc);
	UNUSED_PARAM(argv);
	UNUSED_PARAM(error);

	LIBXML_TEST_VERSION // TODO replace with nondestructive behavior. setjmp?..
	xmlInitParser(); /* no return codes here */
	return true;
}

static bool _startXef(const xplStartParamsPtr params, int argc, const char **argv, xmlChar **error)
{
	xefStartupParams xef_params;

	UNUSED_PARAM(params);
	UNUSED_PARAM(argc);
	UNUSED_PARAM(argv);

	if (!xefStartup(&xef_params))
	{
		if (error)
		{
			if (xef_params.error)
				*error = xef_params.error;
			else
				*error = BAD_CAST XPL_STRDUP("external libraries startup failed (unknown error)");
		}
		return false;
	}
	return true;
}

static bool _startXpl(const xplStartParamsPtr params, int argc, const char **argv, xmlChar **error)
{
	xplError err_code;

	UNUSED_PARAM(argc);
	UNUSED_PARAM(argv);

	err_code = xplInitParser(params->verbose);
	if (err_code != XPL_ERR_NO_ERROR)
	{
		if (error)
			*error = xplFormat("error starting interpreter: \"%s\"", xplErrorToString(err_code));
		return false;
	}
	return true;
}

typedef struct _StartStopStep
{
	bool (*start_fn)(const xplStartParamsPtr params, int argc, const char **argv, xmlChar **error);
	void (*stop_fn)(void);
} StartStopStep, *StartStopStepPtr;

static StartStopStep start_stop_steps[] =
{
	{ _startXpr, _stopXpr },
	{ _startXml, xmlCleanupParser },
	{ _startXef, xefShutdown },
	{ _startXpl, xplDoneParser }
};
#define START_STOP_STEP_COUNT (sizeof(start_stop_steps) / sizeof(start_stop_steps[0]))

bool xplStartEngine(const xplStartParamsPtr params, int argc, const char **argv, xmlChar **error)
{
	int i, j;

	for (i = 0; i < (int) START_STOP_STEP_COUNT; i++)
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

	for (i = START_STOP_STEP_COUNT - 1; i >= 0; i--)
		start_stop_steps[i].stop_fn();
}

#ifdef _LEAK_DETECTION
	const bool xplDefaultDebugAllocation = true;
#else
	const bool xplDefaultDebugAllocation = false;
#endif

#ifdef _USE_TCMALLOC
	const bool xplDefaultUseTcmalloc = true;
#else
	const bool xplDefaultUseTcmalloc = false;
#endif

const xplStartParams xplDefaultStartParams =
{
	.xpr_start_flags = XPR_STARTSTOP_EVERYTHING,
	.verbose = false
};
