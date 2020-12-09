/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __setoption_H
#define __setoption_H

#include <libxpl/xplcommand.h>

void xplCmdSetOptionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSetOptionCommand;

#endif
