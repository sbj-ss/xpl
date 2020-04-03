/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __parsexml_H
#define __parsexml_H

#include "Command.h"

void xplCmdParseXmlPrologue(xplCommandInfoPtr commandInfo);
void xplCmdParseXmlEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplParseXmlCommand;

#endif
