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

void xprShutdownApp(int code)
{
	if (shutdown_func)
		shutdown_func();
	else
		exit(code);
}
