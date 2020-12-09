/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __crash_H
#define __crash_H

#include <libxpl/xplcommand.h>

void xplCmdCrashEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplCrashCommand;

#endif
