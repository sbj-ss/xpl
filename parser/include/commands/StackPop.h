/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __stackpop_H
#define __stackpop_H

#include <libxpl/xplcommand.h>

void xplCmdStackPopPrologue(xplCommandInfoPtr commandInfo);
void xplCmdStackPopEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplStackPopCommand;

#endif
