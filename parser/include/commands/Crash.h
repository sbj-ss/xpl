/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __crash_H
#define __crash_H

#include "Command.h"

void xplCmdCrashPrologue(xplCommandInfoPtr commandInfo);
void xplCmdCrashEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplCrashCommand;

#endif
