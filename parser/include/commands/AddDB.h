/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __adddb_H
#define __adddb_H

#include <libxpl/xplcommand.h>

void xplCmdAddDBPrologue(xplCommandInfoPtr commandInfo);
void xplCmdAddDBEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplAddDBCommand;

#endif
