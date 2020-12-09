/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __currentmacro_H
#define __currentmacro_H

#include <libxpl/xplcommand.h>

void xplCmdCurrentMacroEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplCurrentMacroCommand;

#endif
