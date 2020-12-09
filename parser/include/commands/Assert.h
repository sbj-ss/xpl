/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __assert_H
#define __assert_H

#include <libxpl/xplcommand.h>

void xplCmdAssertEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplAssertCommand;

#endif
