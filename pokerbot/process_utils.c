#include <stdio.h>
#include "process_utils.h"

int start_app_process(_TCHAR *prog, _TCHAR *cmd, STARTUPINFO *si, PROCESS_INFORMATION *pi)
{
	int ret = 0;
	_TCHAR *name, *cmd_l;

	if (!si || !pi)
	{
		_ftprintf(stderr, "Error at start_app_process() - empty info pointers\n");
		return 1;
	}

	/* Allocate memory because cmd_l can be changed by CreateProcess() */
	cmd_l = malloc(_tcslen(prog) + 1 + _tcslen(cmd) + 1);
	_tcscpy(cmd_l, prog);
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
		NULL,							// Use parent's starting directory 
		si,								// Pointer to STARTUPINFO structure
		pi)								// Pointer to PROCESS_INFORMATION structure
		)
	{
		_ftprintf(stderr, _T("Error at start_app_process() - CreateProcess failed [(%d)]\n"), GetLastError());
		ret = 1;
	}

	if (name = _tcsrchr(prog, _T('\\')))
		++name;	

	if (!ret)	
		log(_T("Process %s started\n\t-> PID[%d] THREAD[%d]\n"), (name != NULL ? name : prog), pi->dwProcessId, pi->dwThreadId);
		
	if (cmd_l)
		free(cmd_l);
	cmd_l = NULL;

	return ret;
}