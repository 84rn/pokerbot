#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#include <Windows.h>
#include <tchar.h>
int start_app_process(_TCHAR *prog, _TCHAR *cmd, STARTUPINFO *si, PROCESS_INFORMATION *pi);


#endif