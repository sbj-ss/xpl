/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __element_H
#define __element_H

#include "Command.h"

void xplCmdElementPrologue(xplCommandInfoPtr commandInfo);
void xplCmdElementEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplElementCommand;

#endif
