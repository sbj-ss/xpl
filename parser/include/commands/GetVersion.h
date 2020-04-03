/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __getversion_H
#define __getversion_H

#include "Command.h"

void xplCmdGetVersionPrologue(xplCommandInfoPtr commandInfo);
void xplCmdGetVersionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplGetVersionCommand;

#endif
