/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __container_H
#define __container_H

#include <libxpl/xplcommand.h>

void xplCmdContainerPrologue(xplCommandInfoPtr commandInfo);
void xplCmdContainerEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplContainerCommand;

#endif
