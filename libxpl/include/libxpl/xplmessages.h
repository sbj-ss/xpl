/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __xplmessages_H
#define __xplmessages_H

#include "Configuration.h"
#include <stdbool.h>
#include <libxml/tree.h>
#include <libxml/xmlstring.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _xplMsgType
{
	XPL_MSG_UNKNOWN = -1,
	XPL_MSG_DEBUG = 0,
	XPL_MSG_INFO,
	XPL_MSG_WARNING,
	XPL_MSG_ERROR,
	XPL_MSG_INTERNAL_ERROR
} xplMsgType;

XPLPUBFUN xplMsgType XPLCALL
	xplMsgTypeFromString(const xmlChar *severity, bool allowInternalError);
XPLPUBFUN xmlChar* XPLCALL
	xplFormatMessage(const xmlChar *fmt, ...);
/* The same via va_list - for wrappers */
XPLPUBFUN xmlChar* XPLCALL
	xplVFormatMessage(const xmlChar *fmt, va_list args);
XPLPUBFUN void XPLCALL
	xplDisplayMessage(xplMsgType msgType, const xmlChar *fmt, ...);
XPLPUBFUN void XPLCALL
	xplDisplayWarning(const xmlNodePtr carrier, const xmlChar *fmt, ...);
/* eats msg */
XPLPUBFUN xmlNodePtr XPLCALL
	xplCreateSimpleErrorNode(const xmlDocPtr doc, const xmlChar *msg, const xmlChar *src);
XPLPUBFUN xmlNodePtr XPLCALL
	xplCreateErrorNode(const xmlNodePtr cmd, const xmlChar *fmt_msg, ...);
XPLPUBFUN void XPLCALL
	xplStackTrace(const xmlNodePtr startPoint);

#define DISPLAY_INTERNAL_ERROR_MESSAGE() xplDisplayMessage(XPL_MSG_INTERNAL_ERROR,\
	BAD_CAST "please contact the developer. Function %s, file %s, line %d",\
	__FUNCTION__, __FILE__, __LINE__);

XPLPUBFUN bool XPLCALL
	xplInitMessages(void);
XPLPUBFUN void XPLCALL
	xplCleanupMessages(void);

#ifdef __cplusplus
}
#endif
#endif
