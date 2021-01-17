#include <libxpl/abstraction/xpr.h>
#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlerror.h>

static int active_subsystems = 0;

int _vscprintf (const char *format, va_list pargs)
{
	int retval;
	va_list argcopy;

	va_copy(argcopy, pargs);
	retval = vsnprintf(NULL, 0, format, argcopy);
	va_end(argcopy);
	return retval;
}

int _scprintf(const char *format, ...)
{
	int retval;
	va_list args;

	va_start(args, format);
	retval = vsnprintf(NULL, 0, format, args);
	va_end(args);
	return retval;
}

static xmlChar *convertToFSPath(const xmlChar *path)
{
	xmlChar *internal_path;

	internal_path = BAD_CAST XPL_STRDUP((char*) path);
	if (!internal_path)
		return NULL;
	xprConvertSlashes(internal_path);
	return internal_path;
}

FILE *xprFOpen(const xmlChar *path, const char *mode)
{
	xmlChar *internal_path;
	FILE *ret;

	if (!path)
		return NULL;
	if (!(internal_path = convertToFSPath(path)))
		return NULL;
	ret = fopen((char*) internal_path, mode);
	XPL_FREE(internal_path);
	return ret;
}

int xprSOpen(const xmlChar *path, int mode, int sharing, int perms)
{
	xmlChar *internal_path;
	int ret;

	if (!path)
		return -1;
	if (!(internal_path = convertToFSPath(path)))
		return -1;
	ret = open((char*) internal_path, mode | sharing, perms);
	XPL_FREE(internal_path);
	return ret;
}

bool xprCheckFilePresence(const xmlChar *path)
{
	xmlChar *internal_path;
	struct stat st;
	int stat_ret;

	if (!path)
		return false;
	if (!(internal_path = convertToFSPath(path)))
		return false;
	stat_ret = stat((char*) path, &st);
	XPL_FREE(internal_path);
	/* in EACCES etc error cases file isn't available anyway */
	return !stat_ret;
}

bool xprEnsurePathExistence(const xmlChar *path)
{
	xmlChar *internal_path, *slash_pos, tmp;
	bool create = false;

	if (!path)
		return false;
	if (!(internal_path = convertToFSPath(path)))
		return false;
	slash_pos = internal_path;
	while ((slash_pos = (xmlChar*) xmlStrchr(slash_pos, '/')))
	{
		tmp = *slash_pos;
		*slash_pos = 0;
		if (create)
		{
			if (mkdir((char*) internal_path, 0755) != 0)
			{
				XPL_FREE(internal_path);
				return false;
			}
		} else if (!xprCheckFilePresence(internal_path))
			create = true;
		*slash_pos = tmp;
		slash_pos++;
	}
	XPL_FREE(internal_path);
	return true;
}

xmlChar* xprGetProgramPath(void)
{
    char buf[FILENAME_MAX], *last_slash;
    xmlChar *ret;

    *buf = 0;
    ssize_t size = readlink("/proc/self/exe", buf, FILENAME_MAX - 1);
    if (size == -1)
    	return NULL;
    buf[size] = 0;
    last_slash = strrchr(buf, '/');
    if (last_slash)
    	size = last_slash - buf;
    ret = XPL_MALLOC(size + 1);
    if (!ret)
    	return NULL;
    memcpy(ret, buf, size);
    ret[size] = 0;
    return ret;
}

bool xprMutexInit(XPR_MUTEX *m)
{
	return !pthread_mutex_init(m, NULL);
}

bool xprMutexAcquire(XPR_MUTEX *m)
{
	return !pthread_mutex_lock(m);
}

bool xprMutexRelease(XPR_MUTEX *m)
{
	return !pthread_mutex_unlock(m);
}

bool xprMutexCleanup(XPR_MUTEX *m)
{
	return !pthread_mutex_destroy(m);
}

bool xprSemaphoreInit(XPR_SEMAPHORE *s, int initial_value)
{
	return !sem_init(s, 0, initial_value);
}

bool xprSemaphoreAcquire(XPR_SEMAPHORE *s)
{
	return !sem_wait(s);
}

bool xprSemaphoreRelease(XPR_SEMAPHORE *s)
{
	return !sem_post(s);
}

bool xprSemaphoreCleanup(XPR_SEMAPHORE *s)
{
	return !sem_destroy(s);
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
	return pthread_self();
}

XPR_SHARED_OBJECT_HANDLE xprLoadSharedObject(xmlChar *path)
{
	return dlopen((char*) path, RTLD_NOW);
}

void xprUnloadSharedObject(XPR_SHARED_OBJECT_HANDLE handle)
{
	dlclose(handle);
}

void* xprGetProcAddress(XPR_SHARED_OBJECT_HANDLE handle, char *name)
{
	return dlsym(handle, name);
}

void xprSleep(int ms)
{
    struct timespec ts;
    int res;

    if (ms < 0)
        return;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);
}

bool xprParseCommandLine(void)
{
	// TODO
	return true;
}

void xprSpawnProcessCopy(void)
{
	// TODO
}

void xprWaitForThreads(XPR_THREAD_HANDLE *handles, int count)
{
	UNUSED_PARAM(handles);
	UNUSED_PARAM(count);
	// TODO
}

void xprSetConsoleColor(int color)
{
	// TODO check terminal type
	color = color & 0x0F;
	if (color > 7)
		xmlGenericError(xmlGenericErrorContext, "\e[3%d;1m", color & 0x07);
	else
		xmlGenericError(xmlGenericErrorContext, "\e[3%dm", color);
}

void xprResetConsoleColor()
{
	xmlGenericError(xmlGenericErrorContext, "\e[0m");
}

void xprDebugBreak(void)
{
	raise(SIGTRAP);
}

bool xprIsDebuggerPresent(void)
{
	/* TODO: this will break gdb attaching to live process */
    static bool checked = false;
    static bool debugged = false;

    if (!checked)
    {
        if (ptrace(PTRACE_TRACEME, 0, 1, 0) < 0)
             debugged = true;
        else
        	ptrace(PTRACE_DETACH, 0, 1, 0);
        checked = true;
   }
   return debugged;
}

bool xprStartup(int what)
{
	what &= ~active_subsystems;
	if (what & XPR_STARTSTOP_LOW_LEVEL)
	{
		NOOP();
	}
	if (what & XPR_STARTSTOP_CONSOLE)
	{
		NOOP();
	}
	if (what & XPR_STARTSTOP_PHOENIX_TECH)
	{
		NOOP();
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
		NOOP();
	}
	if (what & XPR_STARTSTOP_CONSOLE)
	{
		NOOP();
	}
	if (what & XPR_STARTSTOP_PHOENIX_TECH)
	{
		NOOP();
	}
	active_subsystems &= ~what;
}
