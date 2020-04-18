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

#define XPR_FS_PATH_DELIM '\\'
#define XPR_FS_PATH_INVERSE_DELIM '/'
#define XPR_FS_PATH_DELIM_STR "\\"
#define XPR_FS_PATH_INVERSE_DELIM_STR "/"

#define XPR_FS_ENCODING "utf-8"
#define XPR_CONSOLE_ENCODING "utf-8"

/* the same for console */
#define XPR_CONSOLE_CHAR char
#define XPR_MK_CONSOLE_CHAR(x) x
#define XPR_MK_CONSOLE_STRING(x) x

#define XPR_CONSOLE_STRCAT(s, t) strcat(s, t)
#define XPR_CONSOLE_STRCHR(s, c) strchr(s, c)
#define XPR_CONSOLE_STRCMP(s1, s2) strcmp(s1, s2)
#define XPR_CONSOLE_STRCPY(d, s) strcpy(d, s)
#define XPR_CONSOLE_STRDUP(s) strdup(s)
#define XPR_CONSOLE_STRLEN(s) strlen(s)
#define XPR_CONSOLE_STRNCMP(d, s, l) strncmp(d, s, l)
#define XPR_CONSOLE_STRNCPY(d, s, l) strncpy(d, s, l)
#define XPR_CONSOLE_STRRCHR(s, c) strrchr(s, c)

/*
#define XPR_FILE_FIND_DATA struct _wfinddata64_t
#define XPR_FILE_FIND_HANDLE intptr_t
#define XPR_FILE_FIND_FIRST(s, d) _wfindfirst64(s, d)
#define XPR_FILE_FIND_NEXT(h, d) _wfindnext64(h, d)
#define XPR_FILE_FIND_CLOSE(h) _findclose(h)
#define XPR_FILE_FIND_DONE(x) ((x) == -1)
*/
/*
#define XPR_FILE_IS_HIDDEN(fdp) ((fdp)->attrib & _A_HIDDEN)
#define XPR_FILE_IS_READONLY(fdp) ((fdp)->attrib & _A_RDONLY)
#define XPR_FILE_IS_SYSTEM(fdp) ((fdp)->attrib & _A_SYSTEM)
#define XPR_FILE_IS_ARCHIVE(fdp) ((fdp)->attrib & _A_ARCH)
#define XPR_FILE_IS_DIRECTORY(fdp) ((fdp)->attrib & _A_SUBDIR)
#define XPR_FILE_SIZE_FROM_FDP(fdp) ((fdp)->size)
#define XPR_FILE_ATIME_FROM_FDP(fdp) (&(fdp)->time_access)
#define XPR_FILE_CTIME_FROM_FDP(fdp) (&(fdp)->time_create)
#define XPR_FILE_WTIME_FROM_FDP(fdp) (&(fdp)->time_write)
#define XPR_FILE_NAME_FROM_FDP(fdp) ((fdp)->name)
*/

#define XPR_FILE_UNLINK(s) unlink(s)
#define XPR_FILE_UNLINK_FAILED(x) ((x) == -1)

#define O_BINARY 0
#define O_TEXT 0
/*
#define XPR_FILE_COPY(src, dst, failIfExists) CopyFileW(src, dst, failIfExists)
#define XPR_FILE_COPY_FAILED(x) (!(x))
#define XPR_FILE_MOVE(src, dst, failIfExists) MoveFileExW(src, dst, MOVEFILE_COPY_ALLOWED | (failIfExists?0:MOVEFILE_REPLACE_EXISTING))
#define XPR_FILE_MOVE_FAILED(x) (!(x))
#define XPR_MKDIR(s) _wmkdir(s)
#define XPR_MKDIR_FAILED(x) ((x) == -1)
#define XPR_STAT(fn, buf) _wstat64(fn, buf)
*/
/*
#define XPR_GET_OS_ERROR() GetLastError()
*/
#define XPR_SHARED_OBJECT_EXT (BAD_CAST ".so")
#define XPR_SHARED_OBJECT_HANDLE void*

/* synchronization primitives */
#define XPR_MUTEX pthread_mutex_t
#define XPR_SEMAPHORE sem_t

#define XPR_THREAD_HANDLE pthread_t
#define XPR_THREAD_ROUTINE_CALL
#define XPR_THREAD_ROUTINE_PARAM void*
#define XPR_THREAD_ROUTINE_RESULT void*
#define XPR_START_THREAD(handle, thr_func, param) pthread_create(&handle, )
#define XPR_START_THREAD_SUSPENDED(handle, thr_func, param) CreateThread(NULL, 0, thr_func, param, CREATE_SUSPENDED, NULL)
#define XPR_SUSPEND_THREAD(tid) SuspendThread(tid)
#define XPR_RESUME_THREAD(tid) ResumeThread(tid)
#define XPR_EXIT_THREAD(code) ExitThread(code)

#define XPR_THREAD_ID pthread_t
#define XPR_THREAD_ID_FORMAT "%08lX"
