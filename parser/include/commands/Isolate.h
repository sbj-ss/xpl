/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __isolate_H
#define __isolate_H

#include <libxpl/xplcommand.h>

void xplCmdIsolateEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplIsolateCommand;

#endif
