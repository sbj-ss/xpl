/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __stackisempty_H
#define __stackisempty_H

#include <libxpl/xplcommand.h>

void xplCmdStackIsEmptyEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplStackIsEmptyCommand;

#endif
