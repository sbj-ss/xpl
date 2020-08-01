/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __register_H
#define __register_H

#include "Configuration.h"

#ifdef __cplusplus
extern "C" {
#endif

XPLPUBFUN bool XPLCALL
	xplRegisterBuiltinCommands();
XPLPUBFUN void XPLCALL
	xplUnregisterBuiltinCommands();
#ifdef __cplusplus
}
#endif
#endif
