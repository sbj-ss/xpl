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
#define XPL_SESSION_ID_SIZE 16 /* in raw bytes */
typedef struct _xplSession* xplSessionPtr;

/* manager-level functions */
XPLPUBFUN xplSessionPtr XPLCALL
	xplSessionCreateOrGetShared(const xmlChar *id);
XPLPUBFUN xplSessionPtr XPLCALL
	xplSessionCreateWithAutoId(void);
XPLPUBFUN xplSessionPtr XPLCALL
	xplSessionLookup(const xmlChar *id);
XPLPUBFUN xplSessionPtr XPLCALL
	xplSessionCopy(xplSessionPtr src, bool local_dest);
XPLPUBFUN void XPLCALL
	xplSessionDeleteShared(const xmlChar *id);
XPLPUBFUN void XPLCALL
	xplCleanupStaleSessions(void);

/* session-level functions */
XPLPUBFUN xplSessionPtr XPLCALL
	xplSessionCreateLocal(void);
XPLPUBFUN void XPLCALL
	xplSessionFreeLocal(xplSessionPtr session);
XPLPUBFUN bool XPLCALL
	xplSessionSetObject(xplSessionPtr session, const xmlNodePtr cur, const xmlChar *name);
XPLPUBFUN xmlNodePtr XPLCALL
	xplSessionGetObject(const xplSessionPtr session, const xmlChar *name);
XPLPUBFUN xmlNodePtr XPLCALL
	xplSessionAccessObject(const xplSessionPtr session, const xmlChar *name);
XPLPUBFUN xmlNodePtr XPLCALL
	xplSessionGetAllObjects(const xplSessionPtr session);
XPLPUBFUN xmlNodePtr XPLCALL
	xplSessionAccessAllObjects(const xplSessionPtr session);
XPLPUBFUN void XPLCALL
	xplSessionUnaccess(const xplSessionPtr session);
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
	xplSessionMarkAsSeen(xplSessionPtr session);

/* start/stop */
XPLPUBFUN bool XPLCALL
	xplSessionManagerInit(void);
XPLPUBFUN void XPLCALL
	xplSessionManagerCleanup(void);

#ifdef __cplusplus
}
#endif
#endif
