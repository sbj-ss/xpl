/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __cw_wrapper_H
#define __cw_wrapper_H

#include <civetweb.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_OPTIONS 50
#define MAX_CONF_FILE_LINE_SIZE 8192
#define CONFIG_FILE "xplweb.conf"

extern int exit_flag;

void cwDie(const char *fmt, ...);
char* cwSDup(const char *str);
void cwVerifyExistence(char **options, const char *option_name, int must_be_dir);
void cwSetAbsolutePath(char *options[], const char *option_name, const char *path_to_civetweb_exe);
void cwSanitizeOptions(char *options[], const char *arg0);

const char* cwGetOption(char **options, const char *option_name);
bool cwSetOption(char **options, const char *name, const char *value);
void cwShowServerName(void);
void cwShowUsageAndExit(const char *exeName);
int cwReadConfigFile(const char *config_file, char **options);
void cwProcessCommandLineArguments(int argc, char *argv[], char **options);
void cwInitSystemInfo(void);
void cwInitServerName(void);
void cwFreeSystemInfo(void);
void cwPrintInfo(void);

struct mg_context* cwStart(int argc, char *argv[], void *user_data, struct mg_callbacks *callbacks);

#ifdef __cplusplus
}
#endif
#endif
