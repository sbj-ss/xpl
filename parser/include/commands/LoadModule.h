/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __load_module_H
#define __load_module_H

#include "Command.h"

void xplCmdLoadModulePrologue(xplCommandInfoPtr commandInfo);
void xplCmdLoadModuleEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplLoadModuleCommand;

#endif
