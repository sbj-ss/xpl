/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __SessionIsPresent_H
#define __SessionIsPresent_H

#include "Command.h"

void xplCmdSessionIsPresentPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSessionIsPresentEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSessionIsPresentCommand;

#endif
