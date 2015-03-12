#ifndef WND_MANAGER_H
#define WND_MANAGER_H

#include "thread_utils.h"
#include <tchar.h>

int wnd_manager_start();
s_thread * wnd_manager_get_thread();
HWND wnd_manager_get_hwnd();
int wnd_manager_terminate();
void wnd_manager_cleanup();

#endif