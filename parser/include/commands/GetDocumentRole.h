/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __getdocumentrole_H
#define __getdocumentrole_H

#include "Command.h"

void xplCmdGetDocumentRolePrologue(xplCommandInfoPtr commandInfo);
void xplCmdGetDocumentRoleEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplGetDocumentRoleCommand;

#endif
