/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __with_H
#define __with_H

#include "Command.h"

void xplCmdWithPrologue(xplCommandInfoPtr commandInfo);
void xplCmdWithEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplWithCommand;

#endif
