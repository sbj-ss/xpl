/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __commentnode_H
#define __commentnode_H

#include "Command.h"

void xplCmdCommentNodePrologue(xplCommandInfoPtr commandInfo);
void xplCmdCommentNodeEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplCommentNodeCommand;

#endif
