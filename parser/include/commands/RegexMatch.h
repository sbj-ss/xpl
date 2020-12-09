/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __regexmatch_H
#define __regexmatch_H

#include <libxpl/xplcommand.h>

void xplCmdRegexMatchEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplRegexMatchCommand;

#endif
