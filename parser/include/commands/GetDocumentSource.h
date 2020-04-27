/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __getdocumentsource_H
#define __getdocumentsource_H

#include <libxpl/xplcommand.h>

void xplCmdGetDocumentSourcePrologue(xplCommandInfoPtr commandInfo);
void xplCmdGetDocumentSourceEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplGetDocumentSourceCommand;

#endif
