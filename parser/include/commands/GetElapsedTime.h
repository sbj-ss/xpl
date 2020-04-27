/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __getelapsedtime_H
#define __getelapsedtime_H

#include <libxpl/xplcommand.h>

void xplCmdGetElapsedTimePrologue(xplCommandInfoPtr commandInfo);
void xplCmdGetElapsedTimeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplGetElapsedTimeCommand;

#endif
