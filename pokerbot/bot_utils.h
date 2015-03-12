#ifndef BOT_UTILS_H
#define BOT_UTILS_H

#include <Windows.h>
#include <tchar.h>

typedef struct s_bot_info *p_bot_info;

int start_bot(_TCHAR *prog, _TCHAR *args);
int wait_for_bot(int how_long);
void clean_after_bot();

STARTUPINFO * get_bot_startup_info();
PROCESS_INFORMATION * get_bot_process_info();
HWND get_bot_main_wnd();

int find_bot_main_wnd();

#endif