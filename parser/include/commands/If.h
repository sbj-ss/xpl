/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __if_H
#define __if_H

#include "Command.h"

void xplCmdIfPrologue(xplCommandInfoPtr commandInfo);
void xplCmdIfEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplIfCommand;

#endif
