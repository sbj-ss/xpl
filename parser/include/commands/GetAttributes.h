/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __getattributes_H
#define __getattributes_H

#include <libxpl/xplcommand.h>

void xplCmdGetAttributesPrologue(xplCommandInfoPtr commandInfo);
void xplCmdGetAttributesEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplGetAttributesCommand;

#endif
