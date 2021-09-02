/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2021     */
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
	xplFormat(PRINTF_FORMAT_STRING(const char *fmt), ...) PRINTF_ARGS(1, 2);
/* The same via va_list - for wrappers */
XPLPUBFUN xmlChar* XPLCALL
	xplVFormatMessage(const char *fmt, va_list args);
XPLPUBFUN void XPLCALL
	xplDisplayMessage(xplMsgType msgType, PRINTF_FORMAT_STRING(const char *fmt), ...) PRINTF_ARGS(2, 3);
XPLPUBFUN void XPLCALL
	xplDisplayWarning(const xmlNodePtr carrier, PRINTF_FORMAT_STRING(const char *fmt), ...) PRINTF_ARGS(2, 3);
/* eats msg */
XPLPUBFUN xmlNodePtr XPLCALL
	xplCreateSimpleErrorNode(const xmlDocPtr doc, const char *msg, const xmlChar *src);
XPLPUBFUN xmlNodePtr XPLCALL
	xplCreateErrorNode(const xmlNodePtr cmd, PRINTF_FORMAT_STRING(const char *fmt_msg), ...) PRINTF_ARGS(2, 3);
XPLPUBFUN void XPLCALL
	xplStackTrace(const xmlNodePtr startPoint);

#define DISPLAY_INTERNAL_ERROR_MESSAGE() xplDisplayMessage(XPL_MSG_INTERNAL_ERROR,\
	"please contact the developer. Function %s, file %s, line %d",\
	__func__, __FILE__, __LINE__);

#define SUCCEED_OR_DIE(f) \
	do { \
		if (!(f)) \
		{ \
			xplDisplayMessage( \
				XPL_MSG_INTERNAL_ERROR, \
				"please contact the developer. Function %s, file %s, line %d. Exiting", \
				__func__, __FILE__, __LINE__ \
			); \
			xprShutdownApp(7); \
		} \
	} while (0)

XPLPUBFUN bool XPLCALL
	xplInitMessages(void);
XPLPUBFUN void XPLCALL
	xplCleanupMessages(void);

#ifdef __cplusplus
}
#endif
#endif
