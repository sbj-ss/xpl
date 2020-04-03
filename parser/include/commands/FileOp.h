/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __FileOp_H
#define __FileOp_H

#include "Command.h"

void xplCmdFileOpPrologue(xplCommandInfoPtr commandInfo);
void xplCmdFileOpEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplFileOpCommand;

#endif
