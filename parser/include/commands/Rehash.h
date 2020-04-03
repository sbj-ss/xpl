/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __rehash_H
#define __rehash_H

#include "Command.h"

void xplCmdRehashPrologue(xplCommandInfoPtr commandInfo);
void xplCmdRehashEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplRehashCommand;

#endif
