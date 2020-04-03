/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __setsamode_H
#define __setsamode_H

#include "Command.h"

void xplCmdSetSaModePrologue(xplCommandInfoPtr commandInfo);
void xplCmdSetSaModeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSetSaModeCommand;

#endif
