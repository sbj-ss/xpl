#include "Common.h"
#include "abstraction/xpr.h"
#include "Utils.h"

#include <objbase.h>
#include <shellapi.h>
#include <WinNT.h>
#include <Psapi.h>
#include <winioctl.h>

XPR_CONSOLE_HANDLE console_input_handle = 0;
XPR_CONSOLE_HANDLE console_output_handle = 0;
XPR_CONSOLE_MODE old_console_mode;

void xprWaitForThreads(XPR_THREAD_HANDLE *handles, int count)
{
	WaitForMultipleObjects((DWORD) count, handles, TRUE, INFINITE);
}

void xprSleep(int ms)
{
	Sleep(ms);
}

xmlChar *xprFormatSysError(int error)
{
	LPWSTR	lpwszBuffer;
	xmlChar *ret = NULL;

	FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		(DWORD) error,
		0,
		(LPWSTR) &lpwszBuffer,
		0,
		NULL
		);
	iconv_string("utf-8", "utf-16le", (const char*) lpwszBuffer, (const char*) lpwszBuffer+wcslen(lpwszBuffer)*sizeof(wchar_t), (char**) &ret, NULL);
	LocalFree(lpwszBuffer);
	return ret;
}

void xprParseCommandLine()
{
	size_t i;
	DWORD pid, wait_result;
	HANDLE process;
	wchar_t *end_ptr;
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
			process = OpenProcess(SYNCHRONIZE, FALSE, pid);
			printf("Waiting for the parent process with id %08X (handle %p) to terminate...", pid, process);
			wait_result = WaitForSingleObject(process, INFINITE);
			CloseHandle(process);
			printf(" done (result: %08X).\n", wait_result);
			Sleep(0); /* release quantum and let the scheduler clean up */
		}
	}
}

void _invalid_crt_param_handler(
	const wchar_t * expression,
	const wchar_t * function, 
	const wchar_t * file, 
	unsigned int line,
	uintptr_t pReserved
) 
{
}

void xprSpawnProcessCopy()
{
	wchar_t *params;
	wchar_t *new_cmdline;
	size_t params_len;
	wchar_t pid_str[9];
	DWORD pid;
	STARTUPINFOW startup_info;
	PROCESS_INFORMATION proc_info;

	printf("! Critical error encountered, restarting the interpreter executable\n");
	printf("! Original command line: %s\n", GetCommandLineA());
	params = GetCommandLineW();
	params_len = wcslen(params);
	new_cmdline = (wchar_t*) xmlMalloc((params_len + 16)*sizeof(wchar_t));
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
	if (!CreateProcessW(NULL, new_cmdline, NULL, NULL, TRUE, 0/*CREATE_NEW_CONSOLE*/, NULL, NULL, &startup_info, &proc_info))
		printf("! Failed to spawn replacement process\n");
	else
		printf("! Respawned process id: %08X\n", proc_info.dwProcessId);
	CloseHandle(proc_info.hThread);
	CloseHandle(proc_info.hProcess);
}

static LONG WINAPI xprUnhandledExceptionFilter(__in struct _EXCEPTION_POINTERS* exceptionInfo)
{
	xprSpawnProcessCopy();
	//ExitProcess(-1);
	return EXCEPTION_EXECUTE_HANDLER;
}

BOOL xprStartup(int what)
{
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
		SetConsoleTitleA((char*) XPL_VERSION_FULL);
		console_input_handle = GetStdHandle(STD_INPUT_HANDLE);
		console_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleMode(console_input_handle, &old_console_mode);
		SetConsoleMode(console_input_handle, ENABLE_QUICK_EDIT_MODE | ENABLE_WINDOW_INPUT);
		XPR_INIT_LOCK(console_interlock);
	}
	if (what & XPR_STARTSTOP_PHOENIX_TECH)
	{
		/* ToDo: ������ ��������� */
#ifdef _USE_PHOENIX_TECH
		SetUnhandledExceptionFilter(xprUnhandledExceptionFilter);
#endif
	}
	return TRUE;
}

BOOL xprShutdown(int what)
{
	if (what & XPR_STARTSTOP_LOW_LEVEL)
	{
		CoUninitialize();
	}
	if (what & XPR_STARTSTOP_CONSOLE)
	{
		SetConsoleMode(console_input_handle, old_console_mode);
	}
	if (what & XPR_STARTSTOP_PHOENIX_TECH)
	{
		SetUnhandledExceptionFilter(NULL);
	}
	return TRUE;
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

BOOL xprCheckPid(int pid)
{
	HANDLE process, self_process;
	wchar_t process_fn[MAX_PATH + 1], self_process_fn[MAX_PATH + 1];

	printf("* checking pid. ours is %08X, input is %08X\n", GetCurrentProcessId(), pid);
	if (pid == GetCurrentProcessId())
		return FALSE;
	process = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION, FALSE, (DWORD) pid);
	if (process) /* if the function failed, we assume the process isn't ours */
	{
		printf("* mapped handle to alive process %p\n", process);
		if (!GetProcessImageFileNameW(process, process_fn, sizeof(process_fn)/sizeof(wchar_t)))
		{
			CloseHandle(process);
			return FALSE; /* already exited?.. */
		}
		self_process = GetCurrentProcess();
		GetProcessImageFileNameW(self_process, self_process_fn, sizeof(self_process_fn)/sizeof(wchar_t));
		CloseHandle(process);
		if (!wcscmp(process_fn, self_process_fn))
			return TRUE;
	}
	return FALSE;
}

void xprDebugBreak(void)
{
	__debugbreak();
}

BOOL xprIsDebuggerPresent(void)
{
	return IsDebuggerPresent()? TRUE: FALSE;
}

BOOL xprCheckFilePresence(const XPR_FS_CHAR *path)
{
	XPR_FS_CHAR *cur_path;
	struct _stat64 stat_buf;
	size_t path_len;
	int stat_ret;

	if (!path)
		return FALSE;
	path_len = XPR_FS_STRLEN(path);
	if (!path_len)
		return FALSE;
	if ((path_len == 2) && (path[1] == ':')) /* "x:", special case */ {
		cur_path = (XPR_FS_CHAR*) malloc(4*sizeof(XPR_FS_CHAR));
		*cur_path = *path;
		cur_path[1] = ':';
		cur_path[2] = XPR_FS_PATH_DELIM;
		cur_path[3] = 0;
	} else if (path[path_len] == XPR_FS_PATH_DELIM) { /* "x:\somedir\" */
		if ((path_len == 3) && (path[1] == ':'))  /* "x:\", special case */
			cur_path = (XPR_FS_CHAR*) path;
		else {
			cur_path = XPR_FS_STRDUP(path);
			cur_path[path_len - 1] = 0; /* remove trailing '\' */
		}
	} else
		cur_path = (XPR_FS_CHAR*) path;
	stat_ret = XPR_STAT(cur_path, &stat_buf);
	if (cur_path != path)
		free(cur_path);
	return !stat_ret;
}

BOOL xprEnsurePathExistence(const XPR_FS_CHAR *path)
{
	XPR_FS_CHAR *slash_pos;
	XPR_FS_CHAR *path_copy;
	XPR_FS_CHAR tmp;

	if (!path)
		return TRUE;
	/* �� ������ ���� */
	path_copy = XPR_FS_STRDUP(path);
	slash_pos = path_copy;
	/* �����, �� ������ ����� ���������� �������� */
	while ((slash_pos = XPR_FS_STRCHR(slash_pos, XPR_FS_PATH_DELIM)))
	{
		tmp = *slash_pos;
		*slash_pos = 0;
		if (!xprCheckFilePresence(path_copy))
			break;
		*slash_pos = tmp;
		slash_pos++;
	}
	if (!slash_pos)
	{
		free(path_copy);
		return TRUE; /* ���� �������� �������� �� ����� */
	}

	while (slash_pos)
	{
		if (XPR_MKDIR_FAILED(XPR_MKDIR(path_copy)))
		{
			free(path_copy);
			return FALSE;
		}
		*slash_pos = tmp;
		slash_pos = XPR_FS_STRCHR(++slash_pos, XPR_FS_PATH_DELIM);
		if (slash_pos)
		{
			tmp = *slash_pos;
			*slash_pos = 0;
		}
	}
	free(path_copy);
	return TRUE;
}

xmlChar* xprGetProgramPath()
{
	DWORD buf_size = MAX_PATH;
	wchar_t *module_fn = (wchar_t*) xmlMalloc(buf_size*sizeof(wchar_t));
	wchar_t *last_slash_pos;
	xmlChar *ret = NULL;

	while (GetModuleFileNameW(NULL, module_fn, buf_size) == buf_size)
	{
		buf_size *= 2;
		module_fn = (wchar_t*) xmlRealloc(module_fn, (size_t) buf_size);
	}
	last_slash_pos = wcsrchr(module_fn, XPR_FS_PATH_DELIM);
	if (last_slash_pos)
		*last_slash_pos = 0;
	else
		return NULL; /* just executable file name, no sense in converting */
	iconv_string("utf-8", XPR_FS_ENCODING, (char*) module_fn, (char*) module_fn + wcslen(module_fn)*sizeof(wchar_t), (char**) &ret, NULL);
	return ret;
}


