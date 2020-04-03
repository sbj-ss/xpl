/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __getparam_H
#define __getparam_H

#include "Command.h"

void xplCmdGetParamPrologue(xplCommandInfoPtr commandInfo);
void xplCmdGetParamEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplGetParamCommand;

#endif
