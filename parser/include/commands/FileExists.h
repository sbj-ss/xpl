/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __fileexists_H
#define __fileexists_H

#include <libxpl/xplcommand.h>

void xplCmdFileExistsPrologue(xplCommandInfoPtr commandInfo);
void xplCmdFileExistsEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplFileExistsCommand;

#endif
