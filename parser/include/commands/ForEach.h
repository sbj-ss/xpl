/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __foreach_H
#define __foreach_H

#include "Command.h"

void xplCmdForEachPrologue(xplCommandInfoPtr commandInfo);
void xplCmdForEachEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplForEachCommand;

#endif
