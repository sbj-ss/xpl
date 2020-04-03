#include "commands/Crash.h"
#include "Utils.h"

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
