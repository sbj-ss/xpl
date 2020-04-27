/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __getsamode_H
#define __getsamode_H

#include <libxpl/xplcommand.h>

void xplCmdGetSaModePrologue(xplCommandInfoPtr commandInfo);
void xplCmdGetSaModeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplGetSaModeCommand;

#endif
