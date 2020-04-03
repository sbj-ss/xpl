/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef _messages_H
#define _messages_H

#include "Configuration.h"
#include "Common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _xplMsgType
{
	xplMsgUnknown = -1,
	xplMsgDebug = 0,
	xplMsgInfo,
	xplMsgWarning,
	xplMsgError,
	xplMsgInternalError
} xplMsgType;

XPLPUBFUN xplMsgType XPLCALL
	xplMsgTypeFromString(xmlChar *severity, BOOL allowInternalError);
XPLPUBFUN xmlChar* XPLCALL
	xplFormatMessage(xmlChar *fmt, ...);
/* The same via va_list - for wrappers */
XPLPUBFUN xmlChar* XPLCALL
	xplVFormatMessage(xmlChar *fmt, va_list args);
XPLPUBFUN void XPLCALL
	xplDisplayMessage(xplMsgType msgType, xmlChar *fmt, ...);
/* eats msg */
XPLPUBFUN xmlNodePtr XPLCALL
	xplCreateSimpleErrorNode(xmlDocPtr doc, xmlChar *msg, const xmlChar *src);
XPLPUBFUN xmlNodePtr XPLCALL
	xplCreateErrorNode(const xmlNodePtr cmd, const xmlChar *fmt_msg, ...);
XPLPUBFUN void XPLCALL
	xplStackTrace(const xmlNodePtr startPoint);

#define DISPLAY_INTERNAL_ERROR_MESSAGE() xplDisplayMessage(xplMsgInternalError,\
	BAD_CAST "please contact the developer. Function %s, file %s, line %d",\
	__FUNCTION__, __FILE__, __LINE__);

XPLPUBFUN BOOL XPLCALL
	xplInitMessages();
XPLPUBFUN void XPLCALL
	xplCleanupMessages();

#ifdef __cplusplus
}
#endif
#endif
