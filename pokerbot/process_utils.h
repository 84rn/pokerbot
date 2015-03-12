#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#include <Windows.h>
#include <tchar.h>

typedef struct s_wnd_id
{
	HWND hwnd;
	DWORD pid;
} wnd_id;

int start_app_process(_TCHAR *prog, _TCHAR *cmd, STARTUPINFO *si, PROCESS_INFORMATION *pi);
BOOL get_process_main_wnd(wnd_id *id);


#endif