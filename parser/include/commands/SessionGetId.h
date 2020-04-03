/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __SessionGetId_H
#define __SessionGetId_H

#include "Command.h"

void xplCmdSessionGetIdPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSessionGetIdEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSessionGetIdCommand;

#endif
