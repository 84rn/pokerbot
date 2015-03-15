#include <stdio.h>
#include "main_bot.h"
#include "prog_opts.h"
#include "basic_log.h"
#include <tchar.h>
#include <limits.h>

void cleanup();
int init();

int _tmain(int argc, _TCHAR *argv[])
{
	int ret;

	if (ret = parse_options(argc, argv))
		return ret;

	if (ret = init())
	{
		_ftprintf(stderr, _T("Error: [%d] init failed\n"), ret);
		return ret;
	}		

	if (ret = main_bot_start())
		return ret;

//	main_bot_terminate();

	WaitForSingleObject(main_bot_get_thread()->handle, INFINITE);

	cleanup();

	return 0;
}

int init()
{
	int ret = 0;

	ret = log_init();
	return ret;
}

void cleanup()
{
	main_bot_cleanup();
	log_cleanup();
	cleanup_options();
}