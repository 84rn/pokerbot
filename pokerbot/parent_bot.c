#include <Windows.h>
#include "parent_bot.h"
#include "wnd_manager.h"
#include "prog_opts.h"
#include "thread_utils.h"
#include "basic_log.h"

static struct s_main_bot
{
	s_thread thread;

} main_bot;


DWORD WINAPI main_bot_proc(LPVOID param);

int main_bot_setup()
{
	int ret;

	log_dbg(_T("Setting up main bot\n"));
	main_bot.thread.proc = main_bot_proc;

	if (ret = create_thread(&main_bot.thread))
		return ret;

	log_dbg(_T("Done.\n"));

	return 0;
}

int main_bot_start()
{
	int ret;

	/* Dispatch main bot thread */
	if (ret = main_bot_setup())
	{
		log_error(_T("cannot initialize main bot\n"));
		return ret;
	}

	WaitForSingleObject(main_bot.thread.ev_started, INFINITE);

	/* Dispatch main process thread */

	return 0;
}

DWORD WINAPI main_bot_proc(LPVOID param)
{
	int terminate = 0;
	DWORD wait;

	SetEvent(main_bot.thread.ev_started);

	/* Start window manager */
	wnd_manager_start();


	/* Say hello */

	while (1)
	{
		wait = WaitForSingleObject(main_bot.thread.ev_terminate, 1);

		if (wait == WAIT_OBJECT_0)
			terminate = 1;

		if (terminate)
			break;
		
	}

	wnd_manager_terminate();
	WaitForSingleObject(wnd_manager_get_thread()->handle, INFINITE);
	
	wnd_manager_cleanup();

	return 0;
}

s_thread * main_bot_get_thread()
{
	return &main_bot.thread;
}

int main_bot_terminate()
{
	return SetEvent(main_bot.thread.ev_terminate);
}

void main_bot_cleanup()
{
	cleanup_thread(&main_bot.thread);
}