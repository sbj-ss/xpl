/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __fatal_H
#define __fatal_H

#include <libxpl/xplcommand.h>

void xplCmdFatalEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplFatalCommand;

#endif
