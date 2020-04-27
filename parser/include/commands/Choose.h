/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __choose_H
#define __choose_H

#include <libxpl/xplcommand.h>

void xplCmdChoosePrologue(xplCommandInfoPtr commandInfo);
void xplCmdChooseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplChooseCommand;

#endif
