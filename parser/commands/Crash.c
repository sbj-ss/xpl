#include <libxpl/xplutils.h>
#include "commands/Crash.h"

void xplCmdCrashPrologue(xplCommandInfoPtr commandInfo)
{
}

void xplCmdCrashEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
	int divisor = 0;
	int dividend = 42;
	*((int*) (size_t) (dividend / divisor)) = dividend / divisor;
}

xplCommand xplCrashCommand = { xplCmdCrashPrologue, xplCmdCrashEpilogue };
