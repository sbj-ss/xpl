/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __shutdown_H
#define __shutdown_H

#include <libxpl/xplcommand.h>

void xplCmdShutdownEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplShutdownCommand;

#endif
