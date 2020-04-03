/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __SessionSetObject_H
#define __SessionSetObject_H

#include "Command.h"

void xplCmdSessionSetObjectPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSessionSetObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSessionSetObjectCommand;

#endif
