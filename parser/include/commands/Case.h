/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __case_H
#define __case_H

#include <libxpl/xplcommand.h>

void xplCmdCasePrologue(xplCommandInfoPtr commandInfo);
void xplCmdCaseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplCaseCommand;

#endif
