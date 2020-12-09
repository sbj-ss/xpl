/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __include_H
#define __include_H

#include <libxpl/xplcommand.h>

void xplCmdIncludeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplIncludeCommand;

#endif
