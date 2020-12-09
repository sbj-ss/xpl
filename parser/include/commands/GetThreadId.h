/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __getthreadid_H
#define __getthreadid_H

#include <libxpl/xplcommand.h>

void xplCmdGetThreadIdEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplGetThreadIdCommand;

#endif
