/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __replicate_H
#define __replicate_H

#include "Command.h"

void xplCmdReplicatePrologue(xplCommandInfoPtr commandInfo);
void xplCmdReplicateEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplReplicateCommand;

#endif
