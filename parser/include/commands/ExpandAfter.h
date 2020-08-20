/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __expandafter_H
#define __expandafter_H

#include <libxpl/xplcommand.h>

void xplCmdExpandAfterPrologue(xplCommandInfoPtr commandInfo);
void xplCmdExpandAfterEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplExpandAfterCommand;

#endif
