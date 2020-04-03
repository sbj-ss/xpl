/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __test_H
#define __test_H

#include "Command.h"

void xplCmdTestPrologue(xplCommandInfoPtr commandInfo);
void xplCmdTestEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplTestCommand;

#endif
