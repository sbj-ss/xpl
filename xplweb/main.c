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

static xmlChar* _getAppType(void)
{
	return APP_TYPE;
}

static void _shutdownServer(int code)
{
	printf("Exiting on signal %d, waiting for all threads to finish...\n", exit_flag);
	exit_flag = 1;
	mg_stop(ctx);
	xplShutdownEngine();
	mg_exit_library();
	xplCleanupMemory();
	fflush(stdout);
	exit(code);
}

int main(int argc, char* argv[])
{
	xmlChar *error;
	struct mg_callbacks callbacks;
	const int features = MG_FEATURES_DEFAULT;

	xplInitMemory(xplDefaultDebugAllocation, xplDefaultUseTcmalloc);
	if (!mg_init_library(features) && features)
		cwDie("Error starting CivetWeb\n");
	cwHandleNonCoreArgs(argc, argv);

	if (!xplStartEngine((const xplStartParamsPtr) &xplDefaultStartParams, argc, (const char**) argv, &error))
	{
		mg_exit_library();
		cwDie("Error starting XPL engine: %s\n", error); /* no need to free error */
	}
	xprSetShutdownFunc(_shutdownServer);
	xplSetGetAppTypeFunc(_getAppType);

	memset(&callbacks, 0, sizeof(callbacks));
	if (!(ctx = cwStart(argc, argv, NULL, &callbacks)))
	{
		xplShutdownEngine();
		mg_exit_library();
		cwDie("Cannot initialize CivetWeb context");
	}

	mg_set_request_handler(ctx, "**.xpl$", serveXpl, NULL);

	xplDisplayMessage(XPL_MSG_INFO, BAD_CAST "XPL web server based on CivetWeb %s started on port(s) [%s], serving directory \"%s\"",
	    mg_version(),
	    mg_get_option(ctx, "listening_ports"),
	    mg_get_option(ctx, "document_root"));
	fflush(stdout);

	doc_root = BAD_CAST mg_get_option(ctx, "document_root");
	xplSetDocRoot(doc_root);

	while (!exit_flag)
		xprSleep(100);
	_shutdownServer(EXIT_SUCCESS);
}
