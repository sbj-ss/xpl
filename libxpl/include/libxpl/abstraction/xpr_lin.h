/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef _IN_XPR_H
#error do not include this file directly, use xpr.h instead
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>

/* VCRT analogs */
int _vscprintf (const char *format, va_list pargs);
int _scprintf(const char *format, ...);

/* FS-specific encoding and functions */
#define XPR_PATH_DELIM '/'
#define XPR_PATH_INVERSE_DELIM '\\'
#define XPR_PATH_DELIM_STR "/"
#define XPR_PATH_INVERSE_DELIM_STR "\\"

#define XPR_FS_ENCODING "utf-8"
#define XPR_CONSOLE_ENCODING "utf-8"

#define O_BINARY 0
#define O_TEXT 0

#define _SH_COMPAT 0
#define _SH_DENYRW 0
#define _SH_DENYWR 0
#define _SH_DENYRD 0
#define _SH_DENYNO 0
#define _SH_SECURE 0

/* synchronization primitives */
#define XPR_MUTEX pthread_mutex_t
#define XPR_SEMAPHORE sem_t

/* threads */
#define XPR_THREAD_HANDLE pthread_t
#define XPR_THREAD_ID pthread_t
#define XPR_THREAD_ID_FORMAT "%08lX"

/* processes */
#define XPR_PROCESS_ID pid_t
#define XPR_PROCESS_ID_FORMAT "%08X"

/* shared objects */
#define XPR_SHARED_OBJECT_EXT (BAD_CAST ".so")
#define XPR_SHARED_OBJECT_HANDLE void*

/* time */
#define XPR_TIME struct timespec

/* OS error */
#define XPR_SYS_ERROR int
