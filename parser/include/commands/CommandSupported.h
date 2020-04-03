/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __commandsupported_H
#define __commandsupported_H

#include "Command.h"

void xplCmdCommandSupportedPrologue(xplCommandInfoPtr commandInfo);
void xplCmdCommandSupportedEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplCommandSupportedCommand;

#endif
