/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __converttodefine_H
#define __converttodefine_H

#include "Command.h"

void xplCmdConvertToDefinePrologue(xplCommandInfoPtr commandInfo);
void xplCmdConvertToDefineEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplConvertToDefineCommand;

#endif
