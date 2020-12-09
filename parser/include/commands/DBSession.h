/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __dbsession_H
#define __dbsession_H

#include <libxpl/xplcommand.h>

void xplCmdDBSessionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplDBSessionCommand;

#endif
