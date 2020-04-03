/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __getoption_H
#define __getoption_H

#include "Command.h"

void xplCmdGetOptionPrologue(xplCommandInfoPtr commandInfo);
void xplCmdGetOptionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplGetOptionCommand;

#endif
