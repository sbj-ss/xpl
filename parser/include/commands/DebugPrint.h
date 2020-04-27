/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __debugprint_H
#define __debugprint_H

#include <libxpl/xplcommand.h>

void xplCmdDebugPrintPrologue(xplCommandInfoPtr commandInfo);
void xplCmdDebugPrintEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplDebugPrintCommand;

#endif
