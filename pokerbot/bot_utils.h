#ifndef BOT_UTILS_H
#define BOT_UTILS_H

#include <Windows.h>
#include <tchar.h>

int start_bot(_TCHAR *prog, _TCHAR *args);
int wait_for_bot(int how_long);
void clean_after_bot();

int get_bot_startup_info(STARTUPINFO **si);
int get_bot_process_info(PROCESS_INFORMATION **pi);

#endif