#include "basic_log.h"

static char enable_log = 1;

void log(_TCHAR *l, ...)
{
	if (enable_log)
	{
		_tprintf("> ");
		va_list vl;
		va_start(vl, l);
		_vtprintf(l, vl);
		va_end(vl);
	}
}
