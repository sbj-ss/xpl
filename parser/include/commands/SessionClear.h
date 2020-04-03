/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __SessionClear_H
#define __SessionClear_H

#include "Command.h"

void xplCmdSessionClearPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSessionClearEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSessionClearCommand;

#endif
