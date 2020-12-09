/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __delete_H
#define __delete_H

#include <libxpl/xplcommand.h>

void xplCmdDeleteEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplDeleteCommand;

#endif
