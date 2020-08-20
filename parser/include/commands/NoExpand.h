/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __noexpand_H
#define __noexpand_H

#include <libxpl/xplcommand.h>

void xplCmdNoExpandPrologue(xplCommandInfoPtr commandInfo);
void xplCmdNoExpandEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplNoExpandCommand;

#endif
