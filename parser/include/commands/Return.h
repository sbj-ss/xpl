/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __return_H
#define __return_H

#include <libxpl/xplcommand.h>

void xplCmdReturnPrologue(xplCommandInfoPtr commandInfo);
void xplCmdReturnEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplReturnCommand;

#endif
