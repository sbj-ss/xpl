/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __valueof_H
#define __valueof_H

#include "Command.h"

void xplCmdValueOfPrologue(xplCommandInfoPtr commandInfo);
void xplCmdValueOfEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplValueOfCommand;

#endif
