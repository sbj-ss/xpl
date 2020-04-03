/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __isdefined_H
#define __isdefined_H

#include "Command.h"

void xplCmdIsDefinedPrologue(xplCommandInfoPtr commandInfo);
void xplCmdIsDefinedEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplIsDefinedCommand;

#endif
