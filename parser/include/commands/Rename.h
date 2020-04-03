/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __rename_H
#define __rename_H

#include "Command.h"

void xplCmdRenamePrologue(xplCommandInfoPtr commandInfo);
void xplCmdRenameEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplRenameCommand;

#endif
