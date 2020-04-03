/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __getapptype_H
#define __getapptype_H

#include "Command.h"

void xplCmdGetAppTypePrologue(xplCommandInfoPtr commandInfo);
void xplCmdGetAppTypeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplGetAppTypeCommand;

#endif
