#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>

int __stdcall DllMain(int reason);

int __stdcall DllMain(int reason)
{
	switch(reason)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	}
	return 1;
}
