/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __switch_H
#define __switch_H

#include "Command.h"

void xplCmdSwitchPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSwitchEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSwitchCommand;

#endif
