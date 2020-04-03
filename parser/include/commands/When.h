/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __when_H
#define __when_H

#include "Command.h"

void xplCmdWhenPrologue(xplCommandInfoPtr commandInfo);
void xplCmdWhenEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplWhenCommand;

#endif
