/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __comment_H
#define __comment_H

#include "Command.h"

void xplCmdCommentPrologue(xplCommandInfoPtr commandInfo);
void xplCmdCommentEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplCommentCommand;

#endif
