/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2014     */
/******************************************/
#ifndef __startthreadsandwait_H
#define __startthreadsandwait_H

#include <libxpl/xplcommand.h>

void xplCmdStartThreadsAndWaitPrologue(xplCommandInfoPtr commandInfo);
void xplCmdStartThreadsAndWaitEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplStartThreadsAndWaitCommand;

#endif
