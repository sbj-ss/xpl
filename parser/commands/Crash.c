#include "commands/Crash.h"

xplCommand xplCrashCommand =
{
	.prologue = xplCmdCrashPrologue,
	.epilogue = xplCmdCrashEpilogue,
	.flags = 0,
	.params_stencil = NULL
};

void xplCmdCrashPrologue(xplCommandInfoPtr commandInfo)
{
	UNUSED_PARAM(commandInfo);
}

void xplCmdCrashEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	int divisor = 0;
	int dividend = 42;

	UNUSED_PARAM(commandInfo);
	UNUSED_PARAM(result);
	*((int*) (size_t) (dividend / divisor)) = dividend / divisor;
}
