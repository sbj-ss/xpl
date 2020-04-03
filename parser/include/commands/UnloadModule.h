/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __unload_module_H
#define __unload_module_H

#include "Command.h"

void xplCmdUnloadModulePrologue(xplCommandInfoPtr commandInfo);
void xplCmdUnloadModuleEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplUnloadModuleCommand;

#endif
