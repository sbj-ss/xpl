/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __suppressmacros_H
#define __suppressmacros_H

#include <libxpl/xplcommand.h>

void xplCmdSuppressMacrosPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSuppressMacrosEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSuppressMacrosCommand;

#endif
