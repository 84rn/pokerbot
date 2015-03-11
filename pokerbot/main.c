#include <stdio.h>
#include "bot_utils.h"
#include <tchar.h>

int _tmain(int argc, TCHAR *argv[])
{
	if (argc != 3)
	{
		_ftprintf(stderr, _T("Usage: %s <program> <parameter>\n"), argv[0]);
		return 1;
	}

	start_bot(argv[1], argv[2]);

	wait_for_bot(INFINITE);
	clean_after_bot();	
}
