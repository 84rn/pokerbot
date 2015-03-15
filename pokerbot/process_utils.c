#include <stdio.h>
#include "process_utils.h"
#include "basic_log.h"

static HWND h;

int process_start_app(_TCHAR *prog, _TCHAR *cmd, process_data *pi)
{
	int ret = 0;
	_TCHAR *name, *cmd_l;
	_TCHAR *path = NULL;

	if (!pi)
	{
		log_err(_T("start_app_process() - empty info pointers"));
		return 1;
	}
	pi->s_info.cb = sizeof(STARTUPINFO);

	/* Allocate memory because cmd_l can be changed by CreateProcess() */
	cmd_l = malloc((_tcslen(prog) + 1 + _tcslen(cmd) + 1) * sizeof(_TCHAR));
	_tcscpy(cmd_l, prog);

	if (name = _tcsrchr(cmd_l, _T('\\')))
	{
		path = malloc((name - cmd_l + 1) * sizeof(_TCHAR));
		_tcsncpy(path, cmd_l, name - cmd_l);
		path[name - cmd_l] = '\0';
		name++;
	}
		
	_tcscat(cmd_l, _T(" "));
	_tcscat(cmd_l, cmd);


	/* Start the child process */
	if (!CreateProcess(prog,			// Module name (use command line)
		cmd_l,							// Command line
		NULL,							// Process handle not inheritable
		NULL,							// Thread handle not inheritable
		FALSE,							// Set handle inheritance to FALSE
		0,								// No creation flags
		NULL,							// Use parent's environment block
		path,							// Use parent's starting directory 
		&pi->s_info,					// Pointer to STARTUPINFO structure
		&pi->p_info)					// Pointer to PROCESS_INFORMATION structure		
		)
	{
		log_err(_T("start_app_process() - CreateProcess failed [(%d)]"), GetLastError());
		ret = 1;
	}

	if (!ret)
		log_dbg(_T("Process %s started -> PID[%#x] THR[%#x]"), (!name ? prog : name), pi->p_info.dwProcessId, pi->p_info.dwThreadId);

	if (cmd_l)
		free(cmd_l);
	cmd_l = NULL;

	if (path)
		free(path);
	path = NULL;

	return ret;
}

BOOL CALLBACK get_process_main_wnd_callback(HWND hwnd, LPARAM lParam)
{
	wnd_id *id = (wnd_id *)lParam;

	DWORD org_pid = id->pid;
	DWORD pid;

	GetWindowThreadProcessId(hwnd, &pid);

	if (pid == org_pid)
	{
		_TCHAR t[1000];
		id->hwnd = hwnd;
		GetWindowText(hwnd, t, 1000);
		log_dbg(_T("Found window: %s [%p]\n"), t, hwnd);
	}

	return TRUE;
}

BOOL get_process_main_wnd(wnd_id *id)
{
	return EnumWindows(get_process_main_wnd_callback, (LPARAM)id);
}