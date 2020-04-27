/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __break_H
#define __break_H

#include <libxpl/xplcommand.h>

void xplCmdBreakPrologue(xplCommandInfoPtr commandInfo);
void xplCmdBreakEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplBreakCommand;

#endif
