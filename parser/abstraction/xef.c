#include <stdarg.h>
#include <libxpl/abstraction/xef.h>
#include <libxpl/abstraction/xefinternal.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplutils.h>

static XEF_GET_ERROR_TEXT_PROTO(Common)
{
	xefCommonErrorMessagePtr real_msg = (xefCommonErrorMessagePtr) msg;
	return xmlStrdup(real_msg->error_text);
}

static XEF_FREE_ERROR_MESSAGE_PROTO(Common)
{
	xefCommonErrorMessagePtr real_msg = (xefCommonErrorMessagePtr) msg;
	if (!msg)
		return;
	if (real_msg->error_text)
		xmlFree(real_msg->error_text);
	xmlFree(msg);
}

xefErrorMessagePtr xefCreateCommonErrorMessage(xmlChar *format, ...)
{
	va_list arg_list;
	xefCommonErrorMessagePtr ret = (xefCommonErrorMessagePtr) xmlMalloc(sizeof(xefCommonErrorMessage));
	if (!ret)
		return NULL;
	ret->header.subsystem = XEF_SUBSYSTEM_COMMON;
	va_start(arg_list, format);
	ret->error_text = xplVFormatMessage(format, arg_list);
	return (xefErrorMessagePtr) ret;
}

#ifdef _XEF_HAS_REGEX
XEF_STARTUP_PROTO(Regex);
XEF_SHUTDOWN_PROTO(Regex);
XEF_GET_ERROR_TEXT_PROTO(Regex);
XEF_FREE_ERROR_MESSAGE_PROTO(Regex);
#endif

#ifdef _XEF_HAS_TRANSPORT
XEF_STARTUP_PROTO(Transport);
XEF_SHUTDOWN_PROTO(Transport);
XEF_GET_ERROR_TEXT_PROTO(Transport);
XEF_FREE_ERROR_MESSAGE_PROTO(Transport);
#endif

#ifdef _XEF_HAS_DB
XEF_STARTUP_PROTO(Database);
XEF_SHUTDOWN_PROTO(Database);
XEF_GET_ERROR_TEXT_PROTO(Database);
XEF_FREE_ERROR_MESSAGE_PROTO(Database);
#endif

#ifdef _XEF_HAS_HTML_CLEANER
XEF_STARTUP_PROTO(HtmlCleaner);
XEF_SHUTDOWN_PROTO(HtmlCleaner);
XEF_GET_ERROR_TEXT_PROTO(HtmlCleaner);
XEF_FREE_ERROR_MESSAGE_PROTO(HtmlCleaner);
#endif

/* Common, Regex, Transport, Database, HtmlCleaner */
static xefStartupProto xef_startup_functions[] = 
{
	NULL,
#ifdef _XEF_HAS_REGEX
	xefStartupRegex,
#else
	NULL,
#endif
#ifdef _XEF_HAS_TRANSPORT
	xefStartupTransport,
#else
	NULL,
#endif
#ifdef _XEF_HAS_DB
	xefStartupDatabase,
#else
	NULL,
#endif
#ifdef _XEF_HAS_HTML_CLEANER
	xefStartupHtmlCleaner
#else
	NULL
#endif
};

static xefShutdownProto xef_shutdown_functions[] =
{
	NULL,
#ifdef _XEF_HAS_REGEX
	xefShutdownRegex,
#else
	NULL,
#endif
#ifdef _XEF_HAS_TRANSPORT
	xefShutdownTransport,
#else
	NULL,
#endif
#ifdef _XEF_HAS_DB
	xefShutdownDatabase,
#else
	NULL,
#endif
#ifdef _XEF_HAS_HTML_CLEANER
	xefShutdownHtmlCleaner
#else
	NULL
#endif
};

static xefGetErrorTextProto xef_get_error_text_functions[] =
{
	xefGetErrorTextCommon,
#ifdef _XEF_HAS_REGEX
	xefGetErrorTextRegex,
#else
	NULL,
#endif
#ifdef _XEF_HAS_TRANSPORT
	xefGetErrorTextTransport,
#else
	NULL,
#endif
#ifdef _XEF_HAS_DB
	xefGetErrorTextDatabase,
#else
	NULL,
#endif
#ifdef _XEF_HAS_HTML_CLEANER
	xefGetErrorTextHtmlCleaner
#else
	NULL
#endif
};

static xefFreeErrorMessageProto xef_free_error_message_functions[] = 
{
	xefFreeErrorMessageCommon,
#ifdef _XEF_HAS_REGEX
	xefFreeErrorMessageRegex,
#else
	NULL,
#endif
#ifdef _XEF_HAS_TRANSPORT
	xefFreeErrorMessageTransport,
#else
	NULL,
#endif
#ifdef _XEF_HAS_DB
	xefFreeErrorMessageDatabase,
#else
	NULL,
#endif
#ifdef _XEF_HAS_HTML_CLEANER
	xefFreeErrorMessageHtmlCleaner
#else
	NULL
#endif
};

static const int xef_function_count = sizeof(xef_startup_functions) / sizeof(xef_startup_functions[0]);

static bool xef_is_started = false;

bool xefStartup(xefStartupParamsPtr params)
{
	int i, j;
	
	if (xef_is_started)
		return true;
	for (i = 0; i < xef_function_count; i++)
	{
		if (!xef_startup_functions[i])
			continue;
		if (!xef_startup_functions[i](params))
		{
			for (j = i - 1; j >= 0; j--)
				if (xef_shutdown_functions[j])
					xef_shutdown_functions[j]();
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
	for (i = xef_function_count - 1; i >= 0; i--)
		if (xef_shutdown_functions[i])
			xef_shutdown_functions[i]();
	xef_is_started = false;
}

xmlChar* xefGetErrorText(xefErrorMessagePtr msg)
{
	xefErrorMessageHeaderPtr header = (xefErrorMessageHeaderPtr) msg;
	if (header->subsystem > XEF_SUBSYSTEM_MAX)
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return NULL;
	}
	return xef_get_error_text_functions[header->subsystem](msg);
}

void xefFreeErrorMessage(xefErrorMessagePtr msg)
{
	xefErrorMessageHeaderPtr header = (xefErrorMessageHeaderPtr) msg;
	if (header->subsystem > XEF_SUBSYSTEM_MAX)
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return;
	}
	xef_free_error_message_functions[header->subsystem](msg);
}

#ifndef _XEF_SPECIFIC_FETCH_PARAMS_CLEAR
void xefFetchParamsClear(xefFetchDocumentParamsPtr params)
{
	if (params->document)
		xmlFree(params->document);
	if (params->encoding)
		xmlFree(params->encoding);
	if (params->real_uri)
		xmlFree(params->real_uri);
}
#endif

#ifndef _XEF_HAS_DB
bool xefDbCheckAvail(const xmlChar* connString, const xmlChar *name, xmlChar **msg)
{
	return false;
}
#endif
