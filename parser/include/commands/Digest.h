/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __digest_H
#define __digest_H

#include <libxpl/xplcommand.h>

void xplCmdDigestPrologue(xplCommandInfoPtr commandInfo);
void xplCmdDigestEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplDigestCommand;

#endif
