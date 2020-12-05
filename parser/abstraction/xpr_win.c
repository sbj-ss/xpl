#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplstring.h>

#include <errno.h>
#include <objbase.h>
#include <psapi.h>
#include <shellapi.h>
#include <winioctl.h>
#include <winnt.h>
#include <libxml/xmlmemory.h>

static HANDLE console_input_handle = 0;
static HANDLE console_output_handle = 0;
static DWORD old_console_mode;
static int active_subsystems = 0;

static WCHAR *convertToFSPath(const xmlChar *path)
{
	WCHAR *internal_path = NULL, *s;

	xstrIconvString(XPR_FS_ENCODING, "utf-8", (char*) path, (char*) path + xmlStrlen(path), (char**) &internal_path, NULL);
	if (!internal_path)
		return NULL;
	s = internal_path;
	while (*s)
	{
		if (*s == L'/')
			*s = L'\\';
		s++;
	}
	return internal_path;
}

FILE *xprFOpen(const xmlChar *path, const char *mode)
{
	WCHAR *internal_path, *w_mode = NULL;
	FILE *ret;

	if (!path)
		return NULL;
	if (!(internal_path = convertToFSPath(path)))
		return NULL;
	if (mode)
	{
		xstrIconvString(XPR_FS_ENCODING, "utf-8", mode, mode + strlen(mode), (char**) w_mode, NULL);
		if (!w_mode)
		{
			XPL_FREE(internal_path);
			return NULL;
		}
	}
	ret = _wfopen(internal_path, w_mode);
	XPL_FREE(internal_path);
	if (w_mode)
		XPL_FREE(w_mode);
	return ret;
}

int xprSOpen(const xmlChar *path, int mode, int sharing, int perms)
{
	WCHAR *internal_path;
	int ret;

	if (!path)
		return -1;
	if (!(internal_path = convertToFSPath(path)))
		return -1;
	errno = _wsopen_s(&ret, internal_path, mode, sharing, perms);
	XPL_FREE(internal_path);
	return ret;
}

static bool _xprCheckFilePresenceW(const WCHAR *path)
{
#if 0
	struct _stat64 stat_buf;
	size_t path_len;

	if (!path_ptr)
		return false;
	path = *path_ptr;
	if (!path)
		return false;
	path_len = wcslen(path);
	if (path_len < 2)
		return false;
	if ((path_len == 2) && (path[1] == ':')) /* path with a drive letter */
	{  // TODO don't realloc!!
		*path_ptr = path = (WCHAR*) XPL_REALLOC(path, 4*sizeof(WCHAR));
		if (!path)
			return NULL;
		path[2] = XPR_FS_PATH_DELIM; /* append trailing slash */
		path[3] = 0;
	} else if ((path[path_len - 1] == XPR_FS_PATH_DELIM) && ((path_len != 3) || (path[1] != ':')))
		path[path_len - 1] = 0; /* remove trailing slash */
	return !_wstat64(path, &stat_buf);
#endif
	return false;
}

bool xprCheckFilePresence(const xmlChar *path)
{
	WCHAR *internal_path = NULL;
	bool exists;

	if (!path)
		return false;
	if (!(internal_path = convertToFSPath(path)))
		return false;
	exists = _xprCheckFilePresenceW(internal_path);
	XPL_FREE(internal_path);
	return exists;
}

bool xprEnsurePathExistence(const xmlChar *path)
{
	WCHAR *slash_pos, *path_copy, tmp;

	if (!path)
		return true;

	xstrIconvString(XPR_FS_ENCODING, "utf-8", (char*) path, (char*) path + xmlStrlen(path), (char**) &path_copy, NULL);
	slash_pos = path_copy;

	while ((slash_pos = wcschr(slash_pos, XPR_FS_PATH_DELIM)))
	{
		tmp = *slash_pos;
		*slash_pos = 0;
		if (!_xprCheckFilePresenceW(path_copy))
			break;
		*slash_pos = tmp;
		slash_pos++;
	}
	if (!slash_pos)
	{
		XPL_FREE(path_copy);
		return true;
	}

	while (slash_pos)
	{
		if (_wmkdir(path_copy) == -1)
		{
			XPL_FREE(path_copy);
			return false;
		}
		*slash_pos = tmp;
		slash_pos = wcschr(++slash_pos, XPR_FS_PATH_DELIM);
		if (slash_pos)
		{
			tmp = *slash_pos;
			*slash_pos = 0;
		}
	}
	XPL_FREE(path_copy);
	return true;
}

xmlChar* xprGetProgramPath()
{
	DWORD buf_size = MAX_PATH;
	WCHAR *module_fn = (WCHAR*) XPL_MALLOC(buf_size*sizeof(WCHAR));
	WCHAR *last_slash_pos;
	xmlChar *ret = NULL;

	while (GetModuleFileNameW(NULL, module_fn, buf_size) == buf_size)
	{
		buf_size *= 2;
		module_fn = (WCHAR*) XPL_REALLOC(module_fn, (size_t) buf_size);
	}
	last_slash_pos = wcsrchr(module_fn, XPR_FS_PATH_DELIM);
	if (last_slash_pos)
		*last_slash_pos = 0;
	else
		return NULL; /* just executable file name, no sense in converting */
	xstrIconvString("utf-8", XPR_FS_ENCODING, (char*) module_fn, (char*) module_fn + wcslen(module_fn)*sizeof(WCHAR), (char**) &ret, NULL);
	return ret;
}

bool xprMutexInit(XPR_MUTEX *m)
{
	if (!m)
		return false;
	*m = CreateMutex(NULL, false, NULL);
	return !!*m;
}

bool xprMutexAcquire(XPR_MUTEX *m)
{
	DWORD result;

	if (!m)
		return false;
	result = WaitForSingleObject(*m, INFINITE);
	return result == WAIT_OBJECT_0;
}

bool xprMutexRelease(XPR_MUTEX *m)
{
	if (!m)
		return false;
	return !!ReleaseMutex(*m);
}

bool xprMutexCleanup(XPR_MUTEX *m)
{
	if (!m)
		return false;
	return !!CloseHandle(*m);
}

bool xprSemaphoreInit(XPR_SEMAPHORE *s, int initial_value)
{
	if (!s)
		return false;
	*s = CreateSemaphore(NULL, initial_value, initial_value, NULL);
	return !!*s;
}

bool xprSemaphoreAcquire(XPR_SEMAPHORE *s)
{
    DWORD result;

	if (!s)
		return false;
	result = WaitForSingleObject(*s, INFINITE);
	return result == WAIT_OBJECT_0;
}

bool xprSemaphoreRelease(XPR_SEMAPHORE *s)
{
	if (!s)
		return false;
	return !!ReleaseSemaphore(*s, 1, NULL);
}

bool xprSemaphoreCleanup(XPR_SEMAPHORE *s)
{
	if (!s)
		return false;
	return !!CloseHandle(*s);
}

void xprInterlockedDecrement(volatile int *value)
{
	__atomic_fetch_sub(value, 1, __ATOMIC_ACQ_REL);
}

void xprInterlockedIncrement(volatile int *value)
{
	__atomic_fetch_add(value, 1, __ATOMIC_ACQ_REL);
}

XPR_THREAD_ID xprGetCurrentThreadId()
{
	return GetCurrentThreadId();
}

XPR_SHARED_OBJECT_HANDLE xprLoadSharedObject(xmlChar *path)
{
	WCHAR *internal_path;

	if (!path)
		return (XPR_SHARED_OBJECT_HANDLE) -1;
	if (!(internal_path = convertToFSPath(path)))
		return (XPR_SHARED_OBJECT_HANDLE) -1;
	return LoadLibraryW(internal_path);
}

void xprUnloadSharedObject(XPR_SHARED_OBJECT_HANDLE handle)
{
	FreeLibrary(handle);
}

void* xprGetProcAddress(XPR_SHARED_OBJECT_HANDLE handle, char *name)
{
	return GetProcAddress(handle, name);
}

void xprSleep(int ms)
{
	Sleep(ms);
}

xmlChar *xprFormatSysError(int error)
{
	LPWSTR	lpwszBuffer;
	xmlChar *ret = NULL;

	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,	NULL, (DWORD) error, 0, (LPWSTR) &lpwszBuffer, 0, NULL);
	xstrIconvString("utf-8", "utf-16le", (const char*) lpwszBuffer, (const char*) lpwszBuffer + wcslen(lpwszBuffer)*sizeof(WCHAR), (char**) &ret, NULL);
	LocalFree(lpwszBuffer);
	return ret;
}

bool xprParseCommandLine()
{
	size_t i;
	DWORD pid, wait_result;
	HANDLE process;
	WCHAR *end_ptr;
	LPWSTR env;
	LPWSTR *argv;
	int argc;

	env = GetCommandLineW();
	argv = CommandLineToArgvW(env, &argc);
	for (i = 1; i < (size_t) argc; i++)
	{
		if (wcsstr(argv[i], L"+wait=") == argv[i])
		{
			pid = wcstoul(argv[i]+6, &end_ptr, 16);
			process = OpenProcess(SYNCHRONIZE, false, pid);
			printf("Waiting for the parent process with id %08lX (handle %p) to terminate...", pid, process);
			wait_result = WaitForSingleObject(process, INFINITE);
			CloseHandle(process);
			printf(" done (result: %08lX).\n", wait_result);
			Sleep(0); /* release quantum and let the scheduler clean up */
		}
	}
	return true;
}

void xprSpawnProcessCopy()
{
	WCHAR *params;
	WCHAR *new_cmdline;
	size_t params_len;
	WCHAR pid_str[9];
	DWORD pid;
	STARTUPINFOW startup_info;
	PROCESS_INFORMATION proc_info;

	printf("! Critical error encountered, restarting the interpreter executable\n");
	printf("! Original command line: %s\n", GetCommandLineA());
	params = GetCommandLineW();
	params_len = wcslen(params);
	new_cmdline = (WCHAR*) XPL_MALLOC((params_len + 16)*sizeof(WCHAR));
	wcscpy(new_cmdline, params);
	wcscat(new_cmdline, L" +wait=");
	pid = GetCurrentProcessId();
	_swprintf(pid_str, L"%08x", pid);
	wcscat(new_cmdline, pid_str);
	ZeroMemory(&startup_info, sizeof(startup_info));
	startup_info.cb = sizeof(startup_info);
	ZeroMemory(&proc_info, sizeof(proc_info));
	wprintf(L"! Respawning process with command line '%s'\n", new_cmdline);
	/* ������� �� ��������, ������� �������� ��������� */
	if (!CreateProcessW(NULL, new_cmdline, NULL, NULL, true, 0/*CREATE_NEW_CONSOLE*/, NULL, NULL, &startup_info, &proc_info))
		printf("! Failed to spawn replacement process\n");
	else
		printf("! Respawned process id: %08lX\n", proc_info.dwProcessId);
	CloseHandle(proc_info.hThread);
	CloseHandle(proc_info.hProcess);
}

void xprWaitForThreads(XPR_THREAD_HANDLE *handles, int count)
{
	WaitForMultipleObjects((DWORD) count, handles, true, INFINITE);
}


void xprSetConsoleColor(int color)
{
	if (console_output_handle)
		SetConsoleTextAttribute(console_output_handle, color);
}

int xprGetPid()
{
	return (int) GetCurrentProcessId();
}

bool xprCheckPid(int pid)
{
	HANDLE process, self_process;
	WCHAR process_fn[MAX_PATH + 1], self_process_fn[MAX_PATH + 1];

	printf("* checking pid. ours is %08lX, input is %08X\n", GetCurrentProcessId(), pid);
	if (pid == GetCurrentProcessId())
		return false;
	process = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION, false, (DWORD) pid);
	if (process) /* if the function failed, we assume the process isn't ours */
	{
		printf("* mapped handle to alive process %p\n", process);
		if (!GetProcessImageFileNameW(process, process_fn, sizeof(process_fn)/sizeof(WCHAR)))
		{
			CloseHandle(process);
			return false; /* already exited?.. */
		}
		self_process = GetCurrentProcess();
		GetProcessImageFileNameW(self_process, self_process_fn, sizeof(self_process_fn)/sizeof(WCHAR));
		CloseHandle(process);
		if (!wcscmp(process_fn, self_process_fn))
			return true;
	}
	return false;
}

void xprDebugBreak(void)
{
	__debugbreak();
}

bool xprIsDebuggerPresent(void)
{
	return IsDebuggerPresent()? true: false;
}

void _invalid_crt_param_handler(
	const WCHAR *expression,
	const WCHAR *function,
	const WCHAR *file,
	unsigned int line,
	uintptr_t pReserved
)
{
}

#ifdef _USE_PHOENIX_TECH
static LONG WINAPI xprUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* exceptionInfo)
{
	xprSpawnProcessCopy();
	//ExitProcess(-1);
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

bool xprStartup(int what)
{
	what &= ~active_subsystems;
	if (what & XPR_STARTSTOP_LOW_LEVEL)
	{
		CoInitializeEx(NULL, COINIT_MULTITHREADED);
		CoInitializeSecurity(
			NULL, 
			-1,                          // COM authentication
			NULL,                        // Authentication services
			NULL,                        // Reserved
			RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
			RPC_C_IMP_LEVEL_IDENTIFY,    // Default Impersonation  
			NULL,                        // Authentication info
			EOAC_NONE,                   // Additional capabilities 
			NULL                         // Reserved
		);
		_set_invalid_parameter_handler(_invalid_crt_param_handler);
	}
	if (what & XPR_STARTSTOP_CONSOLE)
	{
		SetConsoleTitleA((char*) XPL_VERSION_FULL); // TODO W
		console_input_handle = GetStdHandle(STD_INPUT_HANDLE);
		console_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleMode(console_input_handle, &old_console_mode);
		SetConsoleMode(console_input_handle, ENABLE_QUICK_EDIT_MODE | ENABLE_WINDOW_INPUT);
	}
	if (what & XPR_STARTSTOP_PHOENIX_TECH)
	{
		/* ToDo: this is unstable */
#ifdef _USE_PHOENIX_TECH
		SetUnhandledExceptionFilter(xprUnhandledExceptionFilter);
#endif
	}
	active_subsystems |= what;
	return true;
}

int xprGetActiveSubsystems(void)
{
	return active_subsystems;
}

void xprShutdown(int what)
{
	what &= active_subsystems;
	if (what & XPR_STARTSTOP_LOW_LEVEL)
	{
		CoUninitialize();
	}
	if (what & XPR_STARTSTOP_CONSOLE)
	{
		SetConsoleMode(console_input_handle, old_console_mode);
		console_input_handle = console_output_handle = 0;
	}
	if (what & XPR_STARTSTOP_PHOENIX_TECH)
	{
		SetUnhandledExceptionFilter(NULL);
	}
	active_subsystems &= ~what;
}
