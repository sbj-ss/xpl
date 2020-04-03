/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __edge_H
#define __edge_H

#include "Command.h"

void xplCmdEdgePrologue(xplCommandInfoPtr commandInfo);
void xplCmdEdgeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplEdgeCommand;

#endif
