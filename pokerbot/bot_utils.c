#include <stdio.h> 
#include "bot_utils.h"
#include "process_utils.h"
#include "basic_log.h"

static STARTUPINFO bot_startup_info;
static PROCESS_INFORMATION bot_process_info;

int get_bot_startup_info(STARTUPINFO **si)
{
	*si = &bot_startup_info;
	return 0;
}

int get_bot_process_info(PROCESS_INFORMATION **pi)
{
	*pi = &bot_process_info;
	return 0;
}

int start_bot(_TCHAR *prog, _TCHAR *args)
{
	if (!prog || !args)
	{
		_ftprintf(stderr, "Error: no arguments for start_bot()\n");
		return 1;
	}

	log(_T("Starting pokerbot\n\t-> %s %s\n"), prog, args);
	return start_app_process(prog, args, &bot_startup_info, &bot_process_info);
}

int wait_for_bot(int how_long)
{
	return WaitForSingleObject(bot_process_info.hProcess, how_long);
}

void clean_after_bot()
{
	// Close process and thread handles. 
	CloseHandle(bot_process_info.hProcess);
	CloseHandle(bot_process_info.hThread);
}