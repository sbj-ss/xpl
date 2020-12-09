/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __return_H
#define __return_H

#include <libxpl/xplcommand.h>

void xplCmdReturnEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplReturnCommand;

#endif
