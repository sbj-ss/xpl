/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __getprocessingstatus_H
#define __getprocessingstatus_H

#include <libxpl/xplcommand.h>

void xplCmdGetProcessingStatusEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplGetProcessingStatusCommand;

#endif
