/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __changedb_H
#define __changedb_H

#include "Command.h"

void xplCmdChangeDBPrologue(xplCommandInfoPtr commandInfo);
void xplCmdChangeDBEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplChangeDBCommand;

#endif
