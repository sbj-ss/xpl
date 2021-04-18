/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2021     */
/******************************************/
#ifndef __cw_wrapper_H
#define __cw_wrapper_H

#include <civetweb.h>
#include <stdbool.h>
#include "Configuration.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_OPTIONS 50
#define MAX_CONF_FILE_LINE_SIZE 8192
#define CONFIG_FILE "xplweb.conf"
#define XPL_START_FILE_ARG "xplstart"
#define XPL_DEFAULT_START_FILE "start.xpl"

extern int exit_flag;
extern char *start_file;

void XPLCALL /* print a formatted message then exit */
	cwDie(const char *fmt, ...);
void XPLCALL /* "XPL vA.B running on CiverWeb C.D" */
	cwShowServerName(void);
void XPLCALL /* ditto */
	cwShowUsageAndExit(const char *exeName);
void XPLCALL /* system info, XPL libraries and XEF */
	cwPrintInfo(void);
void XPLCALL /* password file modification, info etc */
	cwHandleNonCoreArgs(int argc, char **argv);
struct mg_context* XPLCALL /* process command line and start server */
	cwStart(int argc, char *argv[], void *user_data, struct mg_callbacks *callbacks);

#ifdef __cplusplus
}
#endif
#endif
