/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __serialize_H
#define __serialize_H

#include <libxpl/xplcommand.h>

void xplCmdSerializePrologue(xplCommandInfoPtr commandInfo);
void xplCmdSerializeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSerializeCommand;

#endif
