/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __starttimer_H
#define __starttimer_H

#include <libxpl/xplcommand.h>

void xplCmdStartTimerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplStartTimerCommand;

#endif
