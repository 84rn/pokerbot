#include <stdio.h>
#include "bot_utils.h"
#include "parent_bot.h"
#include "prog_opts.h"
#include "basic_log.h"
#include <tchar.h>
#include <limits.h>

void cleanup();

int _tmain(int argc, _TCHAR *argv[])
{
	int ret;

	if (ret = parse_options(argc, argv))
		return ret;

	log_init();
	
	if (ret = main_bot_start())
		return ret;

	main_bot_terminate();

	WaitForSingleObject(main_bot_get_thread()->handle, INFINITE);

	cleanup();

	return 0;


	/*
	start_bot(argv[1], argv[2]);	

	if (find_bot_main_wnd() == 0)
	{		
		if (IsWindow(get_bot_main_wnd()))
			_tprintf("Y\n");
	}

	wait_for_bot(INFINITE);
	find_bot_main_wnd();
	clean_after_bot();	
	*/
}

void cleanup()
{
	main_bot_cleanup();
	log_cleanup();
	cleanup_options();
}