/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __error_H
#define __error_H

#include <libxpl/xplcommand.h>

void xplCmdErrorEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplErrorCommand;

#endif
