/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __setparam_H
#define __setparam_H

#include "Command.h"

void xplCmdSetParamPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSetParamEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSetParamCommand;

#endif
