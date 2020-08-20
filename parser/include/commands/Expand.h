/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __expand_H
#define __expand_H

#include <libxpl/xplcommand.h>

void xplCmdExpandPrologue(xplCommandInfoPtr commandInfo);
void xplCmdExpandEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplExpandCommand;

#endif
