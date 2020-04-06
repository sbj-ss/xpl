/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __sessionclear_H
#define __sessionclear_H

#include "Command.h"

void xplCmdSessionClearPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSessionClearEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSessionClearCommand;

#endif
