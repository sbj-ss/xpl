/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __unstringer_H
#define __unstringer_H

#include <libxpl/xplcommand.h>

void xplCmdUnstringerPrologue(xplCommandInfoPtr commandInfo);
void xplCmdUnstringerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplUnstringerCommand;

#endif
