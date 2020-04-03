/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __stackisempty_H
#define __stackisempty_H

#include "Command.h"

void xplCmdStackIsEmptyPrologue(xplCommandInfoPtr commandInfo);
void xplCmdStackIsEmptyEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplStackIsEmptyCommand;

#endif
