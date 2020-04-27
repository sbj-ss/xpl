/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __sessiongetid_H
#define __sessiongetid_H

#include <libxpl/xplcommand.h>

void xplCmdSessionGetIdPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSessionGetIdEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSessionGetIdCommand;

#endif
