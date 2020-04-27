/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2019     */
/******************************************/
#ifndef _IN_XPR_H
#error do not include this file directly, use xpr.h instead
#endif

#ifndef _INC_WINDOWS
# include <Windows.h>
#endif
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>
#include <sys/types.h>
/* TODO */
#include <locstdint.h>
#include <direct.h>

/* Работа с файлами и консолью */
#define XPR_PATH_DELIM '\\'
#define XPR_PATH_INVERSE_DELIM '/'
#define XPR_PATH_DELIM_STR "\\"
#define XPR_PATH_INVERSE_DELIM_STR "/"

#define XPR_FS_PATH_DELIM L'\\'
#define XPR_FS_PATH_INVERSE_DELIM L'/'
#define XPR_FS_PATH_DELIM_STR L"\\"
#define XPR_FS_PATH_INVERSE_DELIM_STR "/"

#define XPR_FS_ENCODING "utf-16le"
#define XPR_CONSOLE_ENCODING "cp866"

#define XPR_FS_CHAR wchar_t
#define XPR_MK_FS_CHAR(x) L##x
#define XPR_MK_FS_STRING(x) L##x
#define XPR_CONSOLE_CHAR char
#define XPR_MK_CONSOLE_CHAR(x) x
#define XPR_MK_CONSOLE_STRING(x) x

#define XPR_FS_STRCAT(s, t) wcscat(s, t)
#define XPR_FS_STRCHR(s, c) wcschr(s, c)
#define XPR_FS_STRCMP(s1, s2) wcscmp(s1, s2)
#define XPR_FS_STRCPY(d, s) wcscpy(d, s)
#define XPR_FS_STRDUP(s) _wcsdup(s)
#define XPR_FS_STRLEN(s) wcslen(s)
#define XPR_FS_STRNCPY(d, s, l) wcsncpy(d, s, l)
#define XPR_FS_STRNCMP(d, s, l) wcsncmp(d, s, l)
#define XPR_FS_STRRCHR(s, c) wcsrchr(s, c)

#define XPR_CONSOLE_STRCAT(s, t) strcat(s, t)
#define XPR_CONSOLE_STRCHR(s, c) strchr(s, c)
#define XPR_CONSOLE_STRCMP(s1, s2) strcmp(s1, s2)
#define XPR_CONSOLE_STRCPY(d, s) strcpy(d, s)
#define XPR_CONSOLE_STRDUP(s) strdup(s)
#define XPR_CONSOLE_STRLEN(s) strlen(s)
#define XPR_CONSOLE_STRNCMP(d, s, l) strncmp(d, s, l)
#define XPR_CONSOLE_STRNCPY(d, s, l) strncpy(d, s, l)
#define XPR_CONSOLE_STRRCHR(s, c) strrchr(s, c)

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

#define XPR_FOPEN(s, m) _wfopen(s, L#m)
#define XPR_FCLOSE(f) fclose(f)
#define XPR_FILE_UNLINK(s) _wunlink(s)
#define XPR_FILE_UNLINK_FAILED(x) ((x) == -1)
#define XPR_FILE_SOPEN(hptr, name, mode, sharing, perms) _wsopen_s(hptr, name, mode, sharing, perms)
#define XPR_FILE_CLOSE(handle) _close(handle)
#define XPR_FILE_COPY(src, dst, failIfExists) CopyFileW(src, dst, failIfExists)
#define XPR_FILE_COPY_FAILED(x) (!(x))
#define XPR_FILE_MOVE(src, dst, failIfExists) MoveFileExW(src, dst, MOVEFILE_COPY_ALLOWED | (failIfExists?0:MOVEFILE_REPLACE_EXISTING))
#define XPR_FILE_MOVE_FAILED(x) (!(x))
#define XPR_MKDIR(s) _wmkdir(s)
#define XPR_MKDIR_FAILED(x) ((x) == -1)
#define XPR_STAT(fn, buf) _wstat64(fn, buf)

#define XPR_GET_OS_ERROR() GetLastError()

#define XPR_SHARED_OBJECT_EXT (BAD_CAST ".dll")
#define XPR_SHARED_OBJECT_HANDLE HMODULE

#define XPR_LOAD_SHARED_OBJECT(name) LoadLibraryW(name)
#define XPR_FREE_SHARED_OBJECT(handle) FreeLibrary(handle)
#define XPR_GET_PROC_ADDRESS(handle, name) GetProcAddress(handle, name)

/* Для блокировок уровня объектов */
#define XPR_MUTEX CRITICAL_SECTION
#define XPR_INIT_LOCK(x) InitializeCriticalSection(&x);
#define XPR_ACQUIRE_LOCK(x) EnterCriticalSection(&x);
#define XPR_RELEASE_LOCK(x) LeaveCriticalSection(&x);
#define XPR_CLEANUP_LOCK(x) DeleteCriticalSection(&x);

#define XPR_EVENT_HANDLE HANDLE
#define XPR_CREATE_EVENT(name) CreateEvent(NULL, true, false, name)
#define XPR_SET_EVENT(h) SetEvent(h)
#define XPR_RESET_EVENT(h) ResetEvent(h)
#define XPR_WAIT_EVENT(h, delay) WaitForSingleObject(h, delay);
#define XPR_WAIT_INFINITE INFINITE
#define XPR_DELETE_EVENT(h) CloseHandle(h)

#define XPR_THREAD_HANDLE HANDLE
#define XPR_THREAD_ROUTINE_CALL WINAPI
#define XPR_THREAD_ROUTINE_PARAM LPVOID
#define XPR_THREAD_ROUTINE_RESULT DWORD
#define XPR_START_THREAD(handle, thr_func, param) handle = CreateThread(NULL, 0, thr_func, param, 0, NULL)
#define XPR_START_THREAD_SUSPENDED(handle, thr_func, param) handle = CreateThread(NULL, 0, thr_func, param, CREATE_SUSPENDED, NULL)
#define XPR_SUSPEND_THREAD(tid) SuspendThread(tid)
#define XPR_RESUME_THREAD(tid) ResumeThread(tid)
#define XPR_EXIT_THREAD(code) ExitThread(code)

#define XPR_INTERLOCKED_INCREMENT(x) InterlockedIncrement(x)
#define XPR_INTERLOCKED_DECREMENT(x) InterlockedDecrement(x)

#define XPR_THREAD_ID DWORD
#define XPR_GET_CURRENT_THREAD_ID() GetCurrentThreadId()


#define XPR_CONSOLE_HANDLE HANDLE
#define XPR_CONSOLE_MODE DWORD
