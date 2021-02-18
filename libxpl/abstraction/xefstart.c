#include <stdarg.h>
#include <libxpl/abstraction/xef.h>
#include <libxpl/xplmessages.h>

typedef struct _xefStartStopStep
{
	bool (*start_fn)(xefStartupParamsPtr params);
	void (*stop_fn)(void);
} xefStartStopStep, *xefStartStopStepPtr;

/* Common, Regex, Transport, Database, HtmlCleaner */
static xefStartStopStep xef_start_stop_steps[] =
{
	{ NULL, NULL },
#ifdef _XEF_HAS_REGEX
	{ xefStartupRegex, xefShutdownRegex },
#else
	{ NULL, NULL },
#endif
#ifdef _XEF_HAS_TRANSPORT
	{ xefStartupTransport, xefShutdownTransport },
#else
	{ NULL, NULL },
#endif
#ifdef _XEF_HAS_DB
	{ xefStartupDatabase, xefShutdownDatabase },
#else
	{ NULL, NULL },
#endif
#ifdef _XEF_HAS_HTML_CLEANER
	{ xefStartupHtmlCleaner, xefShutdownHtmlCleaner },
#else
	{ NULL, NULL },
#endif
#ifdef _XEF_HAS_CRYPTO
	{ xefStartupCrypto, xefShutdownCrypto },
#else
	{ NULL, NULL },
#endif
};

#define XEF_START_STOP_STEP_COUNT (sizeof(xef_start_stop_steps) / sizeof(xef_start_stop_steps[0]))

static bool xef_is_started = false;

bool xefStartup(xefStartupParamsPtr params)
{
	int i, j;
	
	if (xef_is_started)
		return true;
	for (i = 0; i < (int) XEF_START_STOP_STEP_COUNT; i++)
	{
		if (!xef_start_stop_steps[i].start_fn)
			continue;
		if (!xef_start_stop_steps[i].start_fn(params))
		{
			for (j = i - 1; j >= 0; j--)
				if (xef_start_stop_steps[j].stop_fn)
					xef_start_stop_steps[j].stop_fn();
			return false;
		}
	}
	xef_is_started = true;
	return true;
}

bool xefIsStarted(void)
{
	return xef_is_started;
}

void xefShutdown(void)
{
	int i;

	if (!xef_is_started)
		return;
	for (i = XEF_START_STOP_STEP_COUNT - 1; i >= 0; i--)
		if (xef_start_stop_steps[i].stop_fn)
			xef_start_stop_steps[i].stop_fn();
	xef_is_started = false;
}

#ifndef _XEF_HAS_DB
bool xefDbCheckAvail(const xmlChar* connString, const xmlChar *name, xmlChar **msg)
{
	return false;
}
#endif
