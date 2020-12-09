/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __restart_H
#define __restart_H

#include <libxpl/xplcommand.h>

void xplCmdRestartEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplRestartCommand;

#endif
