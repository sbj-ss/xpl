/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __xplstart_H
#define __xplstart_H

#include <stdbool.h>
#include <libxml/xmlstring.h>
#include <libxpl/abstraction/xpr.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _xplStartParams
{
	bool use_tcmalloc;
	bool debug_allocation;
	int xpr_start_flags;
	xmlChar *config_file_name;
} xplStartParams, *xplStartParamsPtr;

const xplStartParams xplDefaultStartParams =
{
	SFINIT(.use_tcmalloc, false),
	SFINIT(.debug_allocation, false),
	SFINIT(.xpr_start_flags, XPR_STARTSTOP_EVERYTHING),
	SFINIT(.config_file_name, BAD_CAST "xpl.xml")
};

bool xplStartEngine(const xplStartParamsPtr params, int argc, const char **argv, xmlChar **error);
void xplShutdownEngine(void);

#ifdef __cplusplus
}
#endif
#endif
