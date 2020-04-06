/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __sessionremoveobject_H
#define __sessionremoveobject_H

#include "Command.h"

void xplCmdSessionRemoveObjectPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSessionRemoveObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSessionRemoveObjectCommand;

#endif
