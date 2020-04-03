/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __text_H
#define __text_H

#include "Command.h"

void xplCmdTextPrologue(xplCommandInfoPtr commandInfo);
void xplCmdTextEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplTextCommand;

#endif
