#include <cw.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "Configuration.h"
#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplversion.h>

/* Set by init_server_name */
static char g_server_base_name[128];
/* Default from init_server_name, updated later from the server config */
static const char *g_server_name = "[name not set]";
/* Set by init_system_info() */
static char *g_system_info;
/* Set by process_command_line_arguments() */
static char g_config_file_name[PATH_MAX] = "";

int exit_flag = 0;

void die(const char *fmt, ...)
{
	va_list ap;
	char msg[512] = "";

	va_start(ap, fmt);
	(void) vsnprintf(msg, sizeof(msg) - 1, fmt, ap);
	msg[sizeof(msg) - 1] = 0;
	va_end(ap);
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

char* sdup(const char *str)
{
	size_t len;
	char *p;

	len = strlen(str) + 1;
	p = (char*) malloc(len);

	if (!p)
		die("Cannot allocate %u bytes", (unsigned) len);

	memcpy(p, str, len);
	return p;
}

const char* get_option(char **options, const char *option_name)
{
	int i = 0;
	const char *opt_value = NULL;

	while (options[2*i])
	{
		if (!strcmp(options[2*i], option_name))
		{
			opt_value = options[2*i + 1];
			break;
		}
		i++;
	}
	return opt_value;
}

int set_option(char **options, const char *name, const char *value)
{
	int i, type;
	const struct mg_option *default_options = mg_get_valid_options();
	const char *multi_sep = NULL;

	/* Not an option of main.c, so check if it is a CivetWeb server option */
	type = MG_CONFIG_TYPE_UNKNOWN; /* type "unknown" means "option not found" */
	for (i = 0; default_options[i].name; i++)
	{
		if (!strcmp(default_options[i].name, name))
		{
			type = default_options[i].type;
			break; /* no need to search for another option */
		}
	}

	switch (type)
	{
		case MG_CONFIG_TYPE_UNKNOWN:
			/* unknown option */
			return 0; /* return error */
		case MG_CONFIG_TYPE_NUMBER:
			/* integer number >= 0, e.g. number of threads */
			{
				char *chk = 0;
				unsigned long num = strtoul(value, &chk, 10);
				(void) num; /* do not check value, only syntax */
				if (!chk || *chk || chk == value)
					/* invalid number */
					return 0;
			}
			break;
		case MG_CONFIG_TYPE_STRING:
			/* any text */
			break;
		case MG_CONFIG_TYPE_STRING_LIST:
			/* list of text items, separated by , */
			multi_sep = ",";
			break;
		case MG_CONFIG_TYPE_STRING_MULTILINE:
			/* lines of text, separated by carriage return line feed */
			multi_sep = "\r\n";
			break;
		case MG_CONFIG_TYPE_BOOLEAN:
			/* boolean value, yes or no */
			if (strcmp(value, "yes") && strcmp(value, "no"))
				/* invalid boolean */
				return 0;
			break;
		case MG_CONFIG_TYPE_YES_NO_OPTIONAL:
			/* boolean value, yes or no */
			if (strcmp(value, "yes") && strcmp(value, "no") && strcmp(value, "optional"))
				/* invalid boolean */
				return 0;
			break;
		case MG_CONFIG_TYPE_FILE:
		case MG_CONFIG_TYPE_DIRECTORY:
			/* TODO (low): check this option when it is set, instead of calling verify_existence later */
			break;
		case MG_CONFIG_TYPE_EXT_PATTERN:
			/* list of patterns, separated by | */
			multi_sep = "|";
			break;
		default:
			die("Unknown option type - option %s", name);
	}

	for (i = 0; i < MAX_OPTIONS; i++)
	{
		if (!options[2*i])
		{
			/* Option not set yet. Add new option */
			options[2*i] = sdup(name);
			options[2*i + 1] = sdup(value);
			options[2*i + 2] = NULL;
			break;
		} else if (!strcmp(options[2*i], name)) {
			if (multi_sep) {
				/* Option already set. Append new value. */
				char *s = (char*) malloc(strlen(options[2*i + 1]) + strlen(multi_sep) + strlen(value) + 1);
				if (!s)
					die("Out of memory");
				sprintf(s, "%s%s%s", options[2*i + 1], multi_sep, value);
				free(options[2*i + 1]);
				options[2*i + 1] = s;
			} else {
				/* Option already set. Overwrite */
				free(options[2*i + 1]);
				options[2*i + 1] = sdup(value);
			}
			break;
		}
	}

	if (i == MAX_OPTIONS)
		die("Too many options specified");

	if (!options[2*i])
		die("Out of memory");
	if (!options[2*i + 1])
		die("Illegal escape sequence, or out of memory");

	/* option set correctly */
	return 1;
}

void show_server_name(void)
{
	fprintf(stderr, "XPL v%d.%d - CivetWeb v%s, built on %s\n", XPL_VERSION_MAJOR, XPL_VERSION_MINOR,
	    mg_version(), __DATE__);
}

void show_usage_and_exit(const char *exeName)
{
	const struct mg_option *options;
	int i;

	if (exeName == 0 || *exeName == 0)
		exeName = "civetweb";

	show_server_name();

	fprintf(stderr, "\nUsage:\n");
	fprintf(stderr, "  Start server with a set of options:\n");
	fprintf(stderr, "    %s [config_file]\n", exeName);
	fprintf(stderr, "    %s [-option value ...]\n", exeName);
	fprintf(stderr, "  Show system information:\n");
	fprintf(stderr, "    %s -I\n", exeName);
	fprintf(stderr, "  Add user/change password:\n");
	fprintf(stderr, "    %s -A <htpasswd_file> <realm> <user> <passwd>\n", exeName);
	fprintf(stderr, "  Remove user:\n");
	fprintf(stderr, "    %s -R <htpasswd_file> <realm> <user>\n", exeName);
	fprintf(stderr, "\nOPTIONS:\n");

	options = mg_get_valid_options();
	for (i = 0; options[i].name != NULL; i++) {
		fprintf(stderr,
		        "  -%s %s\n",
		        options[i].name,
		        ((options[i].default_value == NULL)
		             ? "<empty>"
		             : options[i].default_value));
	}
    // TODO add XPR/XPL options
	exit(EXIT_FAILURE);
}

int read_config_file(const char *config_file, char **options)
{
	char line[MAX_CONF_FILE_LINE_SIZE], *p;
	FILE *fp = NULL;
	size_t i, j, line_no = 0;

	/* Open the config file */
	fp = fopen(config_file, "r");
	if (!fp)
		/* Failed to open the file. Keep errno for the caller. */
		return 0;

	/* Load config file settings first */
	fprintf(stdout, "Loading config file %s\n", config_file);

	/* Loop over the lines in config file */
	while (fgets(line, sizeof(line), fp) != NULL)
	{
		if (!line_no && !memcmp(line, "\xEF\xBB\xBF", 3))
			/* strip UTF-8 BOM */
			p = line + 3;
		else
			p = line;
		line_no++;

		/* Ignore empty lines and comments */
		for (i = 0; isspace((unsigned char)p[i]);)
			i++;
		if (p[i] == '#' || !p[i])
			continue;

		/* Skip spaces, \r and \n at the end of the line */
		for (j = strlen(p); (j > 0) && (isspace((unsigned char)p[j - 1]) || iscntrl((unsigned char)p[j - 1]));)
			p[--j] = 0;

		/* Find the space character between option name and value */
		for (j = i; !isspace((unsigned char)p[j]) && p[j];)
			j++;

		/* Terminate the string - then the string at (p+i) contains the option name */
		p[j] = 0;
		j++;

		/* Trim additional spaces between option name and value - then
		 * (p+j) contains the option value */
		while (isspace((unsigned char) p[j]))
			j++;

		/* Set option */
		if (!set_option(options, p + i, p + j))
			fprintf(stderr, "%s: line %d is invalid, ignoring it:\n %s", config_file, (int)line_no, p);
	}

	(void) fclose(fp);

	return 1;
}


void process_command_line_arguments(int argc, char *argv[], char **options)
{
	char *p;
	size_t i, cmd_line_opts_start = 1;
#if defined(CONFIG_FILE2)
	FILE *fp = NULL;
#endif

	/* Should we use a config file ? */
	if ((argc > 1) && (argv[1] != NULL) && (argv[1][0] != '-') && (argv[1][0] != 0))
	{
		/* The first command line parameter is a config file name. */
		snprintf(g_config_file_name, sizeof(g_config_file_name) - 1, "%s", argv[1]);
		cmd_line_opts_start = 2;
	} else if ((p = strrchr(argv[0], XPR_PATH_DELIM)) == NULL) {
		/* No config file set. No path in arg[0] found. Use default file name in the current path. */
		snprintf(g_config_file_name, sizeof(g_config_file_name) - 1, "%s", CONFIG_FILE);
	} else {
		/* No config file set. Path to exe found in arg[0]. Use default file name next to the executable. */
		snprintf(g_config_file_name, sizeof(g_config_file_name) - 1, "%.*s%c%s", (int)(p - argv[0]), argv[0], XPR_FS_PATH_DELIM, CONFIG_FILE);
	}
	g_config_file_name[sizeof(g_config_file_name) - 1] = 0;

#if defined(CONFIG_FILE2)
	fp = fopen(g_config_file_name, "r");

	/* try alternate config file */
	if (fp == NULL) {
		fp = fopen(CONFIG_FILE2, "r");
		if (fp != NULL)
			strcpy(g_config_file_name, CONFIG_FILE2);
	}
	if (fp != NULL)
		fclose(fp);
#endif

	/* read all configurations from a config file */
	if (0 == read_config_file(g_config_file_name, options))
	{
		if (cmd_line_opts_start == 2)
		{
			/* If config file was set in command line and open failed, die. */
			/* Errno will still hold the error from fopen. */
			die("Cannot open config file %s: %s", g_config_file_name, strerror(errno));
		}
		/* Otherwise: CivetWeb can work without a config file */
	}

	/* If we're under MacOS and started by launchd, then the second
	   argument is process serial number, -psn_.....
	   In this case, don't process arguments at all. */
	if (argv[1] == NULL || memcmp(argv[1], "-psn_", 5) != 0)
	{
		/* Handle command line flags. They override config file and default settings. */
		for (i = cmd_line_opts_start; argv[i] != NULL; i += 2)
		{
			if (argv[i][0] != '-' || argv[i + 1] == NULL)
				show_usage_and_exit(argv[0]);
			if (!set_option(options, &argv[i][1], argv[i + 1]))
				fprintf(stderr, "command line option is invalid, ignoring it:\n %s %s\n", argv[i], argv[i + 1]);
		}
	}
}


void init_system_info(void)
{
	int len = mg_get_system_info(NULL, 0);
	if (len > 0) {
		g_system_info = (char*) malloc((unsigned)len + 1);
		(void) mg_get_system_info(g_system_info, len + 1);
	} else
		g_system_info = sdup("Not available");
}


void init_server_name(void)
{
	assert((strlen(mg_version()) + 12 + xmlStrlen(XPL_VERSION_FULL)) < sizeof(g_server_base_name));
	snprintf(g_server_base_name, sizeof(g_server_base_name), "%s - CivetWeb V%s", XPL_VERSION_FULL, mg_version());
	//g_server_name = g_server_base_name;
}


void free_system_info(void)
{
	free(g_system_info);
}

void verify_existence(char **options, const char *optionName, int mustBeDir)
{
	const char *path = get_option(options, optionName);

	if (path && !xprCheckFilePresence(BAD_CAST path, mustBeDir))
		die("Invalid path for %s: [%s]: (%s). Make sure that path is either "
		    "absolute, or it is relative to xplweb executable.",
		    optionName,
		    path,
		    strerror(errno));
}

void set_absolute_path(char *options[], const char *option_name, const char *path_to_civetweb_exe)
{
	char path[PATH_MAX] = "", absolute[PATH_MAX] = "";
	const char *option_value;
	const char *p;

	/* Check whether option is already set */
	option_value = get_option(options, option_name);

	/* If option is already set and it is an absolute path,
	   leave it as it is -- it's already absolute. */
	if (option_value != NULL && !xprIsPathAbsolute(BAD_CAST option_value))
	{
		/* Not absolute. Use the directory where civetweb executable lives
		   be the relative directory for everything.
		   Extract civetweb executable directory into path. */
		if ((p = strrchr(path_to_civetweb_exe, XPR_PATH_DELIM)) == NULL)
			(void) getcwd(path, sizeof(path));
		else {
			snprintf(path, sizeof(path) - 1, "%.*s", (int) (p - path_to_civetweb_exe), path_to_civetweb_exe);
			path[sizeof(path) - 1] = 0;
		}

		strncat(path, "/", sizeof(path) - strlen(path) - 1);
		strncat(path, option_value, sizeof(path) - strlen(path) - 1);

		/* Absolutize the path, and set the option */
		(void) realpath(path, absolute);
		set_option(options, option_name, absolute);
	}
}

void sanitize_options(char *options[], const char *arg0)
{
	/* Make sure we have absolute paths for files and directories */
	set_absolute_path(options, "document_root", arg0);
	set_absolute_path(options, "put_delete_auth_file", arg0);
	set_absolute_path(options, "cgi_interpreter", arg0);
	set_absolute_path(options, "access_log_file", arg0);
	set_absolute_path(options, "error_log_file", arg0);
	set_absolute_path(options, "global_auth_file", arg0);
	set_absolute_path(options, "ssl_certificate", arg0);

	/* Make extra verification for certain options */
	verify_existence(options, "document_root", 1);
	verify_existence(options, "cgi_interpreter", 0);
	verify_existence(options, "ssl_certificate", 0);
	verify_existence(options, "ssl_ca_path", 1);
	verify_existence(options, "ssl_ca_file", 0);
}

static void signal_handler(int sig_num)
{
	exit_flag = sig_num;
}

static int log_message(const struct mg_connection *conn, const char *message)
{
	fprintf(stderr, "%s\n", message);
	return 0;
}

void print_info(void)
{
	xmlChar *libs, *xef;

	libs = xplLibraryVersionsToString(xplGetCompiledLibraryVersions(), xplGetRunningLibraryVersions());
	xef = xplXefImplementationsToString(xplGetXefImplementations());
	fprintf(stdout, "\n%s (%s)\n%sLibraries:\n%sXEF:\n%s", g_server_base_name, g_server_name, g_system_info, libs, xef);
	XPL_FREE(libs);
	XPL_FREE(xef);
}

struct mg_context* start_civetweb(int argc, char *argv[], void *user_data, struct mg_callbacks *callbacks)
{
	char *options[2*MAX_OPTIONS + 1];
	int i;
	struct mg_context *ctxt;

	/* Initialize options structure */
	memset(options, 0, sizeof(options));
	set_option(options, "document_root", ".");

	/* Update config based on command line arguments */
	process_command_line_arguments(argc, argv, options);

	sanitize_options(options, argv[0]);

	/* Setup signal handler: quit on Ctrl-C */
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);

#if defined(DAEMONIZE)
	/* Daemonize */
	for (i = 0; options[i] != NULL; i++) {
		if (strcmp(options[i], "daemonize") == 0) {
			if (options[i + 1] != NULL) {
				if (mg_strcasecmp(options[i + 1], "yes") == 0) {
					fprintf(stdout, "daemonize.\n");
					if (daemon(0, 0) != 0) {
						fprintf(stdout, "Faild to daemonize main process.\n");
						exit(EXIT_FAILURE);
					}
					FILE *fp;
					if ((fp = fopen(PID_FILE, "w")) == 0) {
						fprintf(stdout, "Can not open %s.\n", PID_FILE);
						exit(EXIT_FAILURE);
					}
					fprintf(fp, "%d", getpid());
					fclose(fp);
				}
			}
			break;
		}
	}
#endif

	/* Start Civetweb */
	if (!callbacks->log_message)
		callbacks->log_message = &log_message;
	ctxt = mg_start(callbacks, user_data, (const char**) options);

	/* mg_start copies all options to an internal buffer.
	 * The options data field here is not required anymore. */
	for (i = 0; options[i] != NULL; i++)
		free(options[i]);

	return ctxt;
}
