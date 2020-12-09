/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __regexsplit_H
#define __regexsplit_H

#include <libxpl/xplcommand.h>

void xplCmdRegexSplitEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplRegexSplitCommand;

#endif
