/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __stackpush_H
#define __stackpush_H

#include "Command.h"

void xplCmdStackPushPrologue(xplCommandInfoPtr commandInfo);
void xplCmdStackPushEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplStackPushCommand;

#endif
