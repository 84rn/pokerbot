#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#include <Windows.h>
#include <tchar.h>

typedef struct s_process_info
{
	STARTUPINFO s_info;
	PROCESS_INFORMATION p_info;
} process_data;

typedef struct s_wnd_id
{
	HWND hwnd;
	DWORD pid;
} wnd_id;

int process_start_app(_TCHAR *prog, _TCHAR *cmd, process_data *pi);
BOOL get_process_main_wnd(wnd_id *id);


#endif