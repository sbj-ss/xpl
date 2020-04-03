/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __setresponse_H
#define __setresponse_H

#include "Command.h"

void xplCmdSetResponsePrologue(xplCommandInfoPtr commandInfo);
void xplCmdSetResponseEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSetResponseCommand;

#endif
