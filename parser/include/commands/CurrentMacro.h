/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __currentmacro_H
#define __currentmacro_H

#include "Command.h"

void xplCmdCurrentMacroPrologue(xplCommandInfoPtr commandInfo);
void xplCmdCurrentMacroEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplCurrentMacroCommand;

#endif
