#include <stdio.h>
#include <stdlib.h>
#include <libxpl/abstraction/xpr.h>

void xprConvertSlashes(xmlChar* path)
{
	while (*path)
	{
		if (*path == XPR_PATH_INVERSE_DELIM)
			*path = XPR_PATH_DELIM;
		path++;
	}
}

xprShutdownFunc shutdown_func = NULL;

xprShutdownFunc xprSetShutdownFunc(xprShutdownFunc f)
{
	xprShutdownFunc tmp = shutdown_func;
	shutdown_func = f;
	return tmp;
}

static bool in_shutdown = false;

void xprShutdownApp(int code)
{
	if (in_shutdown)
	{
		fprintf(stderr, "Attempted to call %s from %s. Exiting immediately.\n", __FUNCTION__, __FUNCTION__);
		exit(EXIT_FAILURE);
	}
	if (shutdown_func)
		shutdown_func(code);
	else
		exit(code);
}
