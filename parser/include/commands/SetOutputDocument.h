/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __setoutputdocument_H
#define __setoutputdocument_H

#include <libxpl/xplcommand.h>

void xplCmdSetOutputDocumentEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplSetOutputDocumentCommand;

#endif
