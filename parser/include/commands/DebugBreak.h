/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __debugbreak_H
#define __debugbreak_H

#include <libxpl/xplcommand.h>

void xplCmdDebugBreakPrologue(xplCommandInfoPtr commandInfo);
void xplCmdDebugBreakEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplDebugBreakCommand;

#endif
