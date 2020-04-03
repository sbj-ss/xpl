/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __error_H
#define __error_H

#include "Command.h"

void xplCmdErrorPrologue(xplCommandInfoPtr commandInfo);
void xplCmdErrorEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplErrorCommand;

#endif
