/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __append_H
#define __append_H

#include "Command.h"

void xplCmdAppendPrologue(xplCommandInfoPtr commandInfo);
void xplCmdAppendEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplAppendCommand;

#endif
