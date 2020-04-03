/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __attribute_H
#define __attribute_H

#include "Command.h"

void xplCmdAttributePrologue(xplCommandInfoPtr commandInfo);
void xplCmdAttributeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplAttributeCommand;

#endif
