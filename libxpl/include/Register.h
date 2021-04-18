/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2021     */
/******************************************/
#ifndef __register_H
#define __register_H

#include "Configuration.h"

#ifdef __cplusplus
extern "C" {
#endif

XPLPUBFUN bool XPLCALL
	xplRegisterBuiltinCommands(void);
XPLPUBFUN void XPLCALL
	xplUnregisterBuiltinCommands(void);
#ifdef __cplusplus
}
#endif
#endif
