#include "basic_log.h"

static char enable_log = 1;
static HANDLE mutex;

HANDLE log_get_mutex()
{
	return mutex;
}

void log_init()
{
	if (!mutex)
		CreateMutex(NULL, FALSE, "log_mutex");
}

void log_cleanup()
{
	CloseHandle(mutex);
}

void log_msg(_TCHAR *l, ...)
{
	if (enable_log)
	{	
		WaitForSingleObject(mutex, INFINITE);
		_tprintf("> ");
		va_list vl;
		va_start(vl, l);
		log(l, vl);
		va_end(vl);
		ReleaseMutex(mutex);
	}
}

void log_error(_TCHAR *l, ...)
{
		WaitForSingleObject(mutex, INFINITE);
		_tprintf("Error: ");
		va_list vl;
		va_start(vl, l);
		log(l, vl);
		va_end(vl);	
		ReleaseMutex(mutex);
}

void log_dbg(_TCHAR *l, ...)
{
	if (enable_log)
	{
		WaitForSingleObject(mutex, INFINITE);
		_tprintf("DEBUG: ");
		va_list vl;
		va_start(vl, l);
		log(l, vl);
		va_end(vl);
		ReleaseMutex(mutex);
	}
}

void log(_TCHAR *l, ...)
{
	

	va_list vl;
	va_start(vl, l);
	_vtprintf(l, vl);
	va_end(vl);
	
	
}
