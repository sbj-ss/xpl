/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __suppress_macros_H
#define __suppress_macros_H

#include "Command.h"

void xplCmdSuppressMacrosPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSuppressMacrosEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSuppressMacrosCommand;

#endif
