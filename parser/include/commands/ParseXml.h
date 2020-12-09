/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __parsexml_H
#define __parsexml_H

#include <libxpl/xplcommand.h>

void xplCmdParseXmlEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result);

extern xplCommand xplParseXmlCommand;

#endif
