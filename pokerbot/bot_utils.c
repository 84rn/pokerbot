#include <stdio.h> 
#include "bot_utils.h"
#include "process_utils.h"
#include "basic_log.h"

typedef struct s_bot_info
{
	HWND main_window;
	STARTUPINFO startup_info;
	PROCESS_INFORMATION process_info;
} t_bot_info;

static t_bot_info  bot_data;


STARTUPINFO * get_bot_startup_info()
{
	return &bot_data.startup_info;
}

PROCESS_INFORMATION * get_bot_process_info()
{
	return &bot_data.process_info;
}

HWND get_bot_main_wnd()
{
	return bot_data.main_window;
}

int start_bot(_TCHAR *prog, _TCHAR *args)
{
	if (!prog || !args)
	{
		_ftprintf(stderr, "Error: no arguments for start_bot()\n");
		return 1;
	}

	log(_T("Starting pokerbot\n\t-> %s %s\n"), prog, args);

	if (start_app_process(prog, args, &bot_data.startup_info, &bot_data.process_info) == 0)
	{
		WaitForInputIdle(bot_data.process_info.hProcess, INFINITE);
		return 0;
	}
	else
		return 1;
}

int wait_for_bot(int how_long)
{
	return WaitForSingleObject(bot_data.process_info.hProcess, how_long);
}

void clean_after_bot()
{
	// Close process and thread handles. 
	CloseHandle(bot_data.process_info.hProcess);
	CloseHandle(bot_data.process_info.hThread);
}

int find_bot_main_wnd()
{
	wnd_id id;
	id.pid = bot_data.process_info.dwProcessId;

	if (get_process_main_wnd(&id))
	{
		bot_data.main_window = id.hwnd;
		return 0;
	}

	return 1;
}