/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __moduleloaded_H
#define __moduleloaded_H

#include <libxpl/xplcommand.h>

void xplCmdModuleLoadedEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplModuleLoadedCommand;

#endif
