#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "Configuration.h"
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplstart.h>
#include <cw_wrapper.h>
#include <glue.h>

static struct mg_context *ctx;

static xmlChar* getAppType(void)
{
	return APP_TYPE;
}

void shutdownServer(void)
{
	xplShutdownEngine();
	// TODO actually wait for threads!
	printf("Exiting on signal %d, waiting for all threads to finish...\n", exit_flag);
	exit_flag = 1;
	mg_stop(ctx);
	fflush(stdout);
	//exit(EXIT_SUCCESS);
}

void handle_non_cw_args(int argc, char **argv)
{
	/* Start option -I:
	 * Show system information and exit
	 * This is very useful for diagnosis. */
	if (argc > 1 && !strcmp(argv[1], "-I"))
	{
		cwPrintInfo();
		exit(EXIT_SUCCESS);
	}
	/* Edit passwords file: Add user or change password, if -A option is specified */
	if (argc > 1 && !strcmp(argv[1], "-A"))
	{
		if (argc != 6)
			cwShowUsageAndExit(argv[0]);
		exit(mg_modify_passwords_file(argv[2], argv[3], argv[4], argv[5])? EXIT_SUCCESS: EXIT_FAILURE);
	}
	/* Edit passwords file: Remove user, if -R option is specified */
	if (argc > 1 && !strcmp(argv[1], "-R"))
	{
		if (argc != 5)
			cwShowUsageAndExit(argv[0]);
		exit(mg_modify_passwords_file(argv[2], argv[3], argv[4], NULL)? EXIT_SUCCESS: EXIT_FAILURE);
	}
	/* Show usage if -h or --help options are specified */
	if (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "-H") || !strcmp(argv[1], "--help")))
		cwShowUsageAndExit(argv[0]);
}

int main(int argc, char* argv[])
{
	xmlChar *error;
	struct mg_callbacks callbacks;

	xplInitMemory(xplDefaultDebugAllocation, xplDefaultUseTcmalloc);
	cwInitServerName();
	cwInitSystemInfo();
	mg_init_library(0);
	handle_non_cw_args(argc, argv);

	if (!xplStartEngine((const xplStartParamsPtr) &xplDefaultStartParams, argc, (const char**) argv, &error))
	{
		printf("Error starting XPL engine: %s\n", error);
		if (error)
			XPL_FREE(error);
		mg_exit_library();
		exit(EXIT_FAILURE);
	}
	xprSetShutdownFunc(shutdownServer);
	xplSetGetAppTypeFunc(getAppType);

	memset(&callbacks, 0, sizeof(callbacks));
	if (!(ctx = cwStart(argc, argv, NULL, &callbacks)))
	{
		xplDisplayMessage(XPL_MSG_ERROR, BAD_CAST "Cannot initialize CivetWeb context");
		xplShutdownEngine();
		mg_exit_library();
		exit(EXIT_FAILURE);
	}

	mg_set_request_handler(ctx, "**.xpl$", serveXpl, NULL);

	xplDisplayMessage(XPL_MSG_INFO, BAD_CAST "XPL web server based on CivetWeb %s started on port(s) [%s], serving directory \"%s\"",
	    mg_version(),
	    mg_get_option(ctx, "listening_ports"),
	    mg_get_option(ctx, "document_root"));
	fflush(stdout);

	app_path = BAD_CAST mg_get_option(ctx, "document_root");
	xplSetDocRoot(app_path);

	while (exit_flag == 0)
		xprSleep(100);
	shutdownServer();
	xprSleep(1000);
	cwFreeSystemInfo();
	mg_exit_library();
	xplCleanupMemory();
	return (EXIT_SUCCESS);
}
