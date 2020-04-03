/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __jsonserialize_H
#define __jsonserialize_H

#include "Command.h"

void xplCmdXJsonSerializePrologue(xplCommandInfoPtr commandInfo);
void xplCmdXJsonSerializeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplXJsonSerializeCommand;

#endif
