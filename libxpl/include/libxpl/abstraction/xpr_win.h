/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2019     */
/******************************************/
#ifndef _IN_XPR_H
#error do not include this file directly, use xpr.h instead
#endif

#ifndef _INC_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <direct.h>

#define XPR_PATH_DELIM '\\'
#define XPR_PATH_INVERSE_DELIM '/'
#define XPR_PATH_DELIM_STR "\\"
#define XPR_PATH_INVERSE_DELIM_STR "/"

#define XPR_FS_PATH_DELIM L'\\'
#define XPR_FS_PATH_INVERSE_DELIM L'/'
#define XPR_FS_PATH_DELIM_STR L"\\"
#define XPR_FS_PATH_INVERSE_DELIM_STR "/"

#define XPR_FS_ENCODING "utf-16le"
#define XPR_CONSOLE_ENCODING "cp866" // TODO unicode console
/*
#define XPR_FILE_FIND_DATA struct _wfinddata64_t
#define XPR_FILE_FIND_HANDLE intptr_t
#define XPR_FILE_FIND_FIRST(s, d) _wfindfirst64(s, d)
#define XPR_FILE_FIND_NEXT(h, d) _wfindnext64(h, d)
#define XPR_FILE_FIND_CLOSE(h) _findclose(h)
#define XPR_FILE_FIND_DONE(x) ((x) == -1)

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

#define XPR_FILE_UNLINK(s) _wunlink(s)
#define XPR_FILE_UNLINK_FAILED(x) ((x) == -1)
*/
/*
#define XPR_FILE_COPY(src, dst, failIfExists) CopyFileW(src, dst, failIfExists)
#define XPR_FILE_COPY_FAILED(x) (!(x))
#define XPR_FILE_MOVE(src, dst, failIfExists) MoveFileExW(src, dst, MOVEFILE_COPY_ALLOWED | (failIfExists?0:MOVEFILE_REPLACE_EXISTING))
#define XPR_FILE_MOVE_FAILED(x) (!(x))
#define XPR_MKDIR(s) _wmkdir(s)
#define XPR_MKDIR_FAILED(x) ((x) == -1)
#define XPR_STAT(fn, buf) _wstat64(fn, buf)

#define XPR_GET_OS_ERROR() GetLastError()
*/
#define XPR_SHARED_OBJECT_EXT (BAD_CAST ".dll")
#define XPR_SHARED_OBJECT_HANDLE HMODULE

#define XPR_MUTEX HANDLE
#define XPR_SEMAPHORE HANDLE

#define XPR_THREAD_HANDLE HANDLE
#define XPR_THREAD_ROUTINE_CALL WINAPI
#define XPR_THREAD_ROUTINE_PARAM LPVOID
#define XPR_THREAD_ROUTINE_RESULT DWORD
#define XPR_START_THREAD(handle, thr_func, param) handle = CreateThread(NULL, 0, thr_func, param, 0, NULL)
#define XPR_START_THREAD_SUSPENDED(handle, thr_func, param) handle = CreateThread(NULL, 0, thr_func, param, CREATE_SUSPENDED, NULL)
#define XPR_SUSPEND_THREAD(tid) SuspendThread(tid)
#define XPR_RESUME_THREAD(tid) ResumeThread(tid)
#define XPR_EXIT_THREAD(code) ExitThread(code)

#define XPR_THREAD_ID DWORD
#define XPR_THREAD_ID_FORMAT "%08lX"

#define XPR_TIME struct timespec
