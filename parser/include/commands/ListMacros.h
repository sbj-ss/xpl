/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __listmacros_H
#define __listmacros_H

#include <libxpl/xplcommand.h>

void xplCmdListMacrosPrologue(xplCommandInfoPtr commandInfo);
void xplCmdListMacrosEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplListMacrosCommand;

#endif
