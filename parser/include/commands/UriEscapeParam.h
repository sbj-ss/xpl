/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __uriescapeparam_H
#define __uriescapeparam_H

#include "Command.h"

void xplCmdUriEscapeParamPrologue(xplCommandInfoPtr commandInfo);
void xplCmdUriEscapeParamEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplUriEscapeParamCommand;

#endif
