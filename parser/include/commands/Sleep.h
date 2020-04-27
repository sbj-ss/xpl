/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __sleep_H
#define __sleep_H

#include <libxpl/xplcommand.h>

void xplCmdSleepPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSleepEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSleepCommand;

#endif
