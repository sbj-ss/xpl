/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __getoutputdocument_H
#define __getoutputdocument_H

#include "Command.h"

void xplCmdGetOutputDocumentPrologue(xplCommandInfoPtr commandInfo);
void xplCmdGetOutputDocumentEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplGetOutputDocumentCommand;

#endif
