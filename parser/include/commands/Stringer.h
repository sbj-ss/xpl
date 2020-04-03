/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __stringer_H
#define __stringer_H

#include "Command.h"

void xplCmdStringerPrologue(xplCommandInfoPtr commandInfo);
void xplCmdStringerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplStringerCommand;

#endif
