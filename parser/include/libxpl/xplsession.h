/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/

#ifndef __xplsession_H
#define __xplsession_H

#include "Configuration.h"
#include <stdbool.h>
#include <time.h>
#include <libxml/tree.h>
#include <libxml/xmlstring.h>

#ifdef __cplusplus
extern "C" {
#endif

/* session object */
#define XPL_SESSION_ID_SIZE 8
typedef struct _xplSession* xplSessionPtr;
/* start/stop */
XPLPUBFUN int XPLCALL
	xplSessionManagerInit(time_t max_lifetime);
XPLPUBFUN void XPLCALL
	xplSessionManagerCleanup(void);
/* manager-level functions */
XPLPUBFUN xplSessionPtr XPLCALL
	xplSessionCreate(const xmlChar *id);
XPLPUBFUN xplSessionPtr XPLCALL
	xplSessionCreateWithAutoId(void);
XPLPUBFUN xplSessionPtr XPLCALL
	xplSessionLookup(const xmlChar *id);
XPLPUBFUN void XPLCALL
	xplDeleteSession(const xmlChar *id);
XPLPUBFUN void XPLCALL
	xplCleanupStaleSessions(void);
/* session-level functions */
XPLPUBFUN int XPLCALL
	xplSessionSetObject(xplSessionPtr session, const xmlNodePtr cur, const xmlChar *name);
XPLPUBFUN xmlNodePtr XPLCALL
	xplSessionGetObject(const xplSessionPtr session, const xmlChar *name);
XPLPUBFUN xmlNodePtr XPLCALL
	xplSessionGetAllObjects(const xplSessionPtr session);
XPLPUBFUN void XPLCALL
	xplSessionRemoveObject(xplSessionPtr session, const xmlChar *name);
XPLPUBFUN void XPLCALL
	xplSessionClear(xplSessionPtr session);
XPLPUBFUN xmlChar* XPLCALL
	xplSessionGetId(xplSessionPtr session);
XPLPUBFUN bool XPLCALL
	xplSessionIsValid(xplSessionPtr session);
XPLPUBFUN bool XPLCALL
	xplSessionGetSaMode(xplSessionPtr session);
XPLPUBFUN bool XPLCALL
	xplSessionSetSaMode(xplSessionPtr session, bool enable, xmlChar *password);
XPLPUBFUN bool XPLCALL
	xplSessionIsJustCreated(xplSessionPtr session);
XPLPUBFUN void XPLCALL
	xplMarkSessionAsSeen(xplSessionPtr session);

#ifdef __cplusplus
}
#endif
#endif
