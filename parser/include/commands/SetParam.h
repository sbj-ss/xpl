/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __setparam_H
#define __setparam_H

#include <libxpl/xplcommand.h>

void xplCmdSetParamEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSetParamCommand;

#endif
