/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __getdb_H
#define __getdb_H

#include "Command.h"

void xplCmdGetDBPrologue(xplCommandInfoPtr commandInfo);
void xplCmdGetDBEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplGetDBCommand;

#endif
