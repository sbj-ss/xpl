/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __inherit_H
#define __inherit_H

#include <libxpl/xplcommand.h>

void xplCmdInheritPrologue(xplCommandInfoPtr commandInfo);
void xplCmdInheritEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplInheritCommand;

#endif
