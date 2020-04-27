/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __save_H
#define __save_H

#include <libxpl/xplcommand.h>

void xplCmdSavePrologue(xplCommandInfoPtr commandInfo);
void xplCmdSaveEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSaveCommand;

#endif
