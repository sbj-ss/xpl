/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __cw_H
#define __cw_H

#include <civetweb.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_OPTIONS 50
#define MAX_CONF_FILE_LINE_SIZE 8192
#define CONFIG_FILE "xplweb.conf"

extern int exit_flag;

void die(const char *fmt, ...);
char* sdup(const char *str);
int is_path_absolute(const char *path); // TODO XPR
void verify_existence(char **options, const char *option_name, int must_be_dir); // TODO XPR
void set_absolute_path(char *options[], const char *option_name, const char *path_to_civetweb_exe);
void sanitize_options(char *options[], const char *arg0);

const char* get_option(char **options, const char *option_name);
int set_option(char **options, const char *name, const char *value);
void show_server_name(void);
void show_usage_and_exit(const char *exeName);
int read_config_file(const char *config_file, char **options);
void process_command_line_arguments(int argc, char *argv[], char **options);
void init_system_info(void);
void init_server_name(void);
void free_system_info(void);
void print_info(void);

struct mg_context* start_civetweb(int argc, char *argv[], void *user_data, struct mg_callbacks *callbacks);

#ifdef __cplusplus
}
#endif
#endif
