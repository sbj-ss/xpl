/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __xpr_H
#define __xpr_H

#include "Configuration.h"
#include <libxml/xmlstring.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _IN_XPR_H
#ifdef _WIN32
#include "xpr_win.h"
#elif __linux__
#include "xpr_lin.h"
#endif
#undef _IN_XPR_H

#define NOOP() ((void)0)

/* ///////////////////////// xpr.c //////////////////////////// */

/* Converts slashes to match current OS. Modifies input string */
XPLPUBFUN void XPLCALL
	xprConvertSlashes(xmlChar* path);

typedef void (*xprShutdownFunc)(void);
XPLPUBFUN xprShutdownFunc XPLCALL
	xprSetShutdownFunc(xprShutdownFunc f);
XPLPUBFUN void XPLCALL
	xprShutdownApp(void);

/* //////////////////// xpr_<platform/>.c //////////////////////// */
XPLPUBFUN FILE* XPLCALL
	xprFOpen(const xmlChar *path, const char *mode);
XPLPUBFUN int XPLCALL
	xprSOpen(const xmlChar *path, int mode, int sharing, int perms);

XPLPUBFUN bool XPLCALL
	xprCheckFilePresence(const xmlChar *path);
XPLPUBFUN bool XPLCALL
	xprEnsurePathExistence(const xmlChar *path);
XPLPUBFUN xmlChar* XPLCALL
	xprGetProgramPath(void);

XPLPUBFUN bool XPLCALL
	xprMutexInit(XPR_MUTEX *m);
XPLPUBFUN bool XPLCALL
	xprMutexAcquire(XPR_MUTEX *m);
XPLPUBFUN bool XPLCALL
	xprMutexRelease(XPR_MUTEX *m);
XPLPUBFUN bool XPLCALL
	xprMutexCleanup(XPR_MUTEX *m);

XPLPUBFUN bool XPLCALL
	xprSemaphoreInit(XPR_SEMAPHORE *s, int initial_value);
XPLPUBFUN bool XPLCALL
	xprSemaphoreAcquire(XPR_SEMAPHORE *s);
XPLPUBFUN bool XPLCALL
	xprSemaphoreRelease(XPR_SEMAPHORE *s);
XPLPUBFUN bool XPLCALL
	xprSemaphoreCleanup(XPR_SEMAPHORE *s);

XPLPUBFUN void XPLCALL
	xprInterlockedDecrement(volatile int *value);
XPLPUBFUN void XPLCALL
	xprInterlockedIncrement(volatile int *value);

XPLPUBFUN XPR_THREAD_ID XPLCALL
	xprGetCurrentThreadId(void);

XPLPUBFUN XPR_SHARED_OBJECT_HANDLE XPLCALL
	xprLoadSharedObject(xmlChar *path);
XPLPUBFUN void XPLCALL
	xprUnloadSharedObject(XPR_SHARED_OBJECT_HANDLE handle);
XPLPUBFUN void* XPLCALL
	xprGetProcAddress(XPR_SHARED_OBJECT_HANDLE handle, char *name);

XPLPUBFUN void XPLCALL 
	xprSleep(int ms);

/* OS error text. Result must be freed. */
XPLPUBFUN xmlChar* XPLCALL
	xprFormatSysError(int error);

XPLPUBFUN bool XPLCALL
	xprParseCommandLine(void);

XPLPUBFUN void XPLCALL
	xprSpawnProcessCopy(void);
XPLPUBFUN void XPLCALL
	xprWaitForThreads(XPR_THREAD_HANDLE *handles, int count);

#define XPR_DEFAULT_CONSOLE_COLOR 0x07

XPLPUBFUN void XPLCALL
	xprSetConsoleColor(int color);
XPLPUBFUN void XPLCALL
	xprResetConsoleColor(void);

XPLPUBFUN int XPLCALL
	xprGetPid(void);
XPLPUBFUN bool XPLCALL
	xprCheckPid(int pid);

XPLPUBFUN void XPLCALL
	xprDebugBreak(void);
XPLPUBFUN bool XPLCALL
	xprIsDebuggerPresent(void);

XPLPUBFUN void XPLCALL
	xprGetTime(XPR_TIME *t);
XPLPUBFUN long XPLCALL
	xprTimeDelta(XPR_TIME *after, XPR_TIME *before);
XPLPUBFUN bool XPLCALL
	xprTimeIsEmpty(XPR_TIME *t);

#define XPR_STARTSTOP_LOW_LEVEL 0x01
#define XPR_STARTSTOP_CONSOLE 0x02
#define XPR_STARTSTOP_PHOENIX_TECH 0x04
#define XPR_STARTSTOP_EVERYTHING 0xFFFFFFFF

XPLPUBFUN bool XPLCALL
	xprStartup(int what);
XPLPUBFUN int XPLCALL
	xprGetActiveSubsystems(void);
XPLPUBFUN void XPLCALL
	xprShutdown(int what);
#ifdef __cplusplus
}
#endif
#endif
