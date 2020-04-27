/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __namespace_H
#define __namespace_H

#include <libxpl/xplcommand.h>

void xplCmdNamespacePrologue(xplCommandInfoPtr commandInfo);
void xplCmdNamespaceEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplNamespaceCommand;

#endif
