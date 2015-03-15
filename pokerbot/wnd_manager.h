#ifndef WND_MANAGER_H
#define WND_MANAGER_H

#include "thread_utils.h"
#include <tchar.h>

// User messages

#define WM_LOG_MAIN_BOT WM_USER + 1
#define WM_LOG_MINI_BOT WM_USER + 2
#define WM_GET_PROCESS_CAPTURE WM_USER + 3
#define WM_GET_PROCESS_ACTIVE_WND WM_USER + 4
#define WM_GET_PROCESS_WND_BY_TITLE WM_USER + 5

int wnd_manager_start();
s_thread * wnd_manager_get_thread();
HWND wnd_manager_get_hwnd();
int wnd_manager_terminate();
void wnd_manager_cleanup();

#endif