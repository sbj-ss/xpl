/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/

#ifndef __sql_H
#define __sql_H

#include "Command.h"

#ifdef __cplusplus
extern "C" {
#endif

void xplCmdSqlPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSqlEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSqlCommand;

#ifdef __cplusplus
}
#endif

#endif
