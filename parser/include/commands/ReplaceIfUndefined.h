/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __replaceifundefined_H
#define __replaceifundefined_H

#include "Command.h"

void xplCmdReplaceIfUndefinedPrologue(xplCommandInfoPtr commandInfo);
void xplCmdReplaceIfUndefinedEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplReplaceIfUndefinedCommand;

#endif
