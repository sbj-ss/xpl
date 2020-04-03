/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __waitforthreads_H
#define __waitforthreads_H

#include "Command.h"

void xplCmdWaitForThreadsPrologue(xplCommandInfoPtr commandInfo);
void xplCmdWaitForThreadsEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplWaitForThreadsCommand;

#endif
