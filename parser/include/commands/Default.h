/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __default_H
#define __default_H

#include "Command.h"

void xplCmdDefaultPrologue(xplCommandInfoPtr commandInfo);
void xplCmdDefaultEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplDefaultCommand;

#endif
