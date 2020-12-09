/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __processinginstruction_H
#define __processinginstruction_H

#include <libxpl/xplcommand.h>

void xplCmdProcessingInstructionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplProcessingInstructionCommand;

#endif
