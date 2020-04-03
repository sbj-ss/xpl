/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __removedb_H
#define __removedb_H

#include "Command.h"

void xplCmdRemoveDBPrologue(xplCommandInfoPtr commandInfo);
void xplCmdRemoveDBEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplRemoveDBCommand;

#endif
