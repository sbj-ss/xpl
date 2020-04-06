/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __stacklocalize_H
#define __stacklocalize_H

#include "Command.h"

void xplCmdStackLocalizePrologue(xplCommandInfoPtr commandInfo);
void xplCmdStackLocalizeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplStackLocalizeCommand;

#endif
