/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __shutdown_H
#define __shutdown_H

#include "Command.h"

void xplCmdShutdownPrologue(xplCommandInfoPtr commandInfo);
void xplCmdShutdownEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplShutdownCommand;

#endif
