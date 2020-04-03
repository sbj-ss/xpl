/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __getdocumentfilename_H
#define __getdocumentfilename_H

#include "Command.h"

void xplCmdGetDocumentFilenamePrologue(xplCommandInfoPtr commandInfo);
void xplCmdGetDocumentFilenameEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplGetDocumentFilenameCommand;

#endif
