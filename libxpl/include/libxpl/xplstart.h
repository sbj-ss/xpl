/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2021     */
/******************************************/
#ifndef __xplstart_H
#define __xplstart_H

#include <stdbool.h>
#include <libxml/xmlstring.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _xplStartParams
{
	int xpr_start_flags;
	bool verbose;
} xplStartParams, *xplStartParamsPtr;

XPLPUBVAR const bool xplDefaultDebugAllocation;
XPLPUBVAR const bool xplDefaultUseTcmalloc;
XPLPUBVAR const xplStartParams xplDefaultStartParams;

XPLPUBFUN bool XPLCALL
	xplInitMemory(bool debugAllocation, bool useTcmalloc);
XPLPUBFUN bool XPLCALL
	xplStartEngine(const xplStartParamsPtr params, int argc, const char **argv, xmlChar **error);
XPLPUBFUN void XPLCALL
	xplShutdownEngine(void);
XPLPUBFUN void XPLCALL
	xplCleanupMemory(void);

#ifdef __cplusplus
}
#endif
#endif
