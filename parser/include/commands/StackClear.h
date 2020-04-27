/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __stackclear_H
#define __stackclear_H

#include <libxpl/xplcommand.h>

void xplCmdStackClearPrologue(xplCommandInfoPtr commandInfo);
void xplCmdStackClearEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplStackClearCommand;

#endif
