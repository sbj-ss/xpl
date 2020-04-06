/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __uriencode_H
#define __uriencode_H

#include "Command.h"

void xplCmdUriEncodePrologue(xplCommandInfoPtr commandInfo);
void xplCmdUriEncodeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplUriEncodeCommand;

#endif
