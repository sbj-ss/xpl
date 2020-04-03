/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __otherwise_H
#define __otherwise_H

#include "Command.h"

void xplCmdOtherwisePrologue(xplCommandInfoPtr commandInfo);
void xplCmdOtherwiseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplOtherwiseCommand;

#endif
