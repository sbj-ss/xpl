#include <libxpl/xplcommand.h>

void xplCmdCrashEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

xplCommand xplCrashCommand =
{
	.prologue = NULL,
	.epilogue = xplCmdCrashEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdCrashEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	int divisor = 0;
	int dividend = 42;

	UNUSED_PARAM(commandInfo);
	UNUSED_PARAM(result);
	*((int*) (size_t) (dividend / divisor)) = dividend / divisor;
}
