/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __sessiongetobject_H
#define __sessiongetobject_H

#include <libxpl/xplcommand.h>

void xplCmdSessionGetObjectEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSessionGetObjectCommand;

#endif
