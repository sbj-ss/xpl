/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __cleanvalue_H
#define __cleanvalue_H

#include "Command.h"

void xplCmdCleanValuePrologue(xplCommandInfoPtr commandInfo);
void xplCmdCleanValueEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplCleanValueCommand;

#endif
