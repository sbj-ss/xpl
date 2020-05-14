/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __xefinternal_H
#define __xefinternal_H

#include "Configuration.h"
#include <stdbool.h>
#include <libxml/xmlstring.h>
#include <libxpl/abstraction/xef.h>

/* subsystems list */
typedef enum _xefSubsystem 
{
	XEF_SUBSYSTEM_COMMON,
	XEF_SUBSYSTEM_REGEX,
	XEF_SUBSYSTEM_TRANSPORT,
	XEF_SUBSYSTEM_DATABASE,
	XEF_SUBSYSTEM_HTML_CLEANER,
	XEF_SUBSYSTEM_MAX = XEF_SUBSYSTEM_HTML_CLEANER
} xefSubsystem;

typedef struct _xefErrorMessageHeader
{
	xefSubsystem subsystem;
	/* implementation-specific fields follow */
} xefErrorMessageHeader, *xefErrorMessageHeaderPtr;

typedef struct _xefCommonErrorMessage
{
	xefErrorMessageHeader header;
	xmlChar *error_text;
} xefCommonErrorMessage, *xefCommonErrorMessagePtr;

xefErrorMessagePtr xefCreateCommonErrorMessage(xmlChar *format, ...);

#define XEF_STARTUP_PROTO(subsys) bool xefStartup##subsys(xefStartupParamsPtr params)
#define XEF_SHUTDOWN_PROTO(subsys) void xefShutdown##subsys(void)
#define XEF_GET_ERROR_TEXT_PROTO(subsys) xmlChar* xefGetErrorText##subsys(xefErrorMessagePtr msg)
#define XEF_FREE_ERROR_MESSAGE_PROTO(subsys) void xefFreeErrorMessage##subsys(xefErrorMessagePtr msg)

typedef bool (*xefStartupProto)(xefStartupParamsPtr params);
typedef void (*xefShutdownProto)(void);
typedef xmlChar* (*xefGetErrorTextProto)(xefErrorMessagePtr msg);
typedef void (*xefFreeErrorMessageProto)(xefErrorMessagePtr msg);

#endif
