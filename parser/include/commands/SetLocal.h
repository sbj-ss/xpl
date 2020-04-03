/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __setlocal_H
#define __setlocal_H

#include "Command.h"

void xplCmdSetLocalPrologue(xplCommandInfoPtr commandInfo);
void xplCmdSetLocalEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSetLocalCommand;

#endif
