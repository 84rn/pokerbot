#include "basic_log.h"

static char enable_log = 1;
static HANDLE mutex;

void log_base(_TCHAR *l, ...);

HANDLE log_get_mutex()
{
	return mutex;
}

int log_init()
{
	int ret = 0;

	if (!mutex)
		ret = ((mutex = CreateMutex(NULL, FALSE, "log_mutex")) ? 0 : GetLastError());

	return ret;
}

void log_cleanup()
{
	CloseHandle(mutex);
}

void log_msg(_TCHAR *l, ...)
{
	if (enable_log)
	{
		va_list vl;
		va_start(vl, l);
		WaitForSingleObject(mutex, INFINITE);
		_tprintf(_T("> "));		
		log_base(l, vl);
		ReleaseMutex(mutex);
		va_end(vl);
	}
}

void log_err(_TCHAR *l, ...)
{
	va_list vl;
	va_start(vl, l);
	WaitForSingleObject(mutex, INFINITE);
	_ftprintf(stderr, "Error: ");	
	_vftprintf(stderr, l, vl);
	_tprintf(_T("\n"));
	ReleaseMutex(mutex);
	va_end(vl);
}

void log_dbg(_TCHAR *l, ...)
{
	if (enable_log)
	{
		WaitForSingleObject(mutex, INFINITE);
		va_list vl;
		va_start(vl, l);
		_tprintf(_T("DEBUG: "));
		log_base(l, vl);
		va_end(vl);
		ReleaseMutex(mutex);
	}
}

void log_base(_TCHAR *l, va_list list)
{
	_vtprintf(l, list);
	_tprintf(_T("\n"));	
}