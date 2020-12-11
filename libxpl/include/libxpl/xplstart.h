/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
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
	xmlChar *config_file_name;
} xplStartParams, *xplStartParamsPtr;

extern const bool xplDefaultDebugAllocation;
extern const bool xplDefaultUseTcmalloc;
extern const xplStartParams xplDefaultStartParams;

bool xplInitMemory(bool debugAllocation, bool useTcmalloc);
bool xplStartEngine(const xplStartParamsPtr params, int argc, const char **argv, xmlChar **error);
void xplShutdownEngine(void);
void xplCleanupMemory(void);

#ifdef __cplusplus
}
#endif
#endif
