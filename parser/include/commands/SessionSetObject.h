/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __sessionsetobject_H
#define __sessionsetobject_H

#include <libxpl/xplcommand.h>

void xplCmdSessionSetObjectPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSessionSetObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSessionSetObjectCommand;

#endif
