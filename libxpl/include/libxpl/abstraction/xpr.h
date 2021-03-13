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

typedef void (*xprShutdownFunc)(int code);
XPLPUBFUN xprShutdownFunc XPLCALL
	xprSetShutdownFunc(xprShutdownFunc f);
XPLPUBFUN void XPLCALL
	xprShutdownApp(int code);

/*//////////////////// xpr_<platform/>.c ////////////////////// */
/* files */
XPLPUBFUN FILE* XPLCALL
	xprFOpen(const xmlChar *path, const char *mode);
XPLPUBFUN int XPLCALL
	xprSOpen(const xmlChar *path, int mode, int sharing, int perms);

XPLPUBFUN bool XPLCALL
	xprIsPathAbsolute(const xmlChar *path);
XPLPUBFUN bool XPLCALL
	xprCheckFilePresence(const xmlChar *path, bool mustBeDir);
XPLPUBFUN bool XPLCALL
	xprEnsurePathExistence(const xmlChar *path);
XPLPUBFUN xmlChar* XPLCALL /* result must be freed */
	xprGetProgramPath(void);
XPLPUBFUN xmlChar* XPLCALL /* result must be freed */
	xprRealPath(xmlChar *path);

/* sync */
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

/* threads */
#define XPR_DECLARE_THREAD_ROUTINE(name, param) XPR_THREAD_RETVAL name(XPR_THREAD_PARAM param)

XPLPUBFUN void XPLCALL
	xprWaitForThreads(XPR_THREAD_HANDLE *handles, int count);
XPLPUBFUN XPR_THREAD_ID XPLCALL
	xprGetCurrentThreadId(void);
XPLPUBFUN XPR_THREAD_HANDLE XPLCALL
	xprStartThread(XPR_THREAD_ROUTINE(f), XPR_THREAD_PARAM p);
XPLPUBFUN void XPLCALL
	xprExitThread(XPR_THREAD_RETVAL code);

/* processes */
XPLPUBFUN XPR_PROCESS_ID XPLCALL
	xprGetPid(void);
XPLPUBFUN bool XPLCALL
	xprCheckPid(XPR_PROCESS_ID pid);
XPLPUBFUN bool XPLCALL
	xprParseCommandLine(void);
XPLPUBFUN void XPLCALL
	xprSpawnProcessCopy(void);

/* shared objects */
XPLPUBFUN XPR_SHARED_OBJECT_HANDLE XPLCALL
	xprLoadSharedObject(xmlChar *path);
XPLPUBFUN void XPLCALL
	xprUnloadSharedObject(XPR_SHARED_OBJECT_HANDLE handle);
XPLPUBFUN void* XPLCALL
	xprGetProcAddress(XPR_SHARED_OBJECT_HANDLE handle, char *name);

/* console */
#define XPR_DEFAULT_CONSOLE_COLOR 0x07

XPLPUBFUN void XPLCALL
	xprSetConsoleColor(int color);
XPLPUBFUN void XPLCALL
	xprResetConsoleColor(void);

/* debugger interaction */
XPLPUBFUN void XPLCALL
	xprDebugBreak(void);
XPLPUBFUN bool XPLCALL
	xprIsDebuggerPresent(void);

/* time */
XPLPUBFUN void XPLCALL
	xprGetTime(XPR_TIME *t);
XPLPUBFUN long XPLCALL
	xprTimeDelta(XPR_TIME *after, XPR_TIME *before);
XPLPUBFUN bool XPLCALL
	xprTimeIsEmpty(XPR_TIME *t);
XPLPUBFUN void XPLCALL
	xprSleep(int ms);

/* low-level error handling */
XPLPUBFUN XPR_SYS_ERROR XPLCALL
	xprGetSysError(void);
XPLPUBFUN xmlChar* XPLCALL
	xprFormatSysError(XPR_SYS_ERROR error); /* result must be freed */

/* init/stop */
#define XPR_STARTSTOP_LOW_LEVEL 0x01
#define XPR_STARTSTOP_CONSOLE 0x02
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
