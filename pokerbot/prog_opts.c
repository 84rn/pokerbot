#include <stdlib.h>
#include <stdio.h>
#include "prog_opts.h"
#include "basic_log.h"

static struct s_prog_options
{
	_TCHAR *app_name;
	_TCHAR *app_params;
} prog_options;

int parse_options(int argc, _TCHAR *argv[])
{
	int app_name_len, app_params_len;

	if (argc < 3)
	{
		print_usage(argv[0]);
		return 1;
	}

	app_name_len = _tcslen(argv[1]);
	app_params_len = _tcslen(argv[2]);

	if (!app_name_len)
	{
		log_error("app process name empty\n");
		return 1;
	}

	prog_options.app_name = malloc((app_name_len + 1) * sizeof(_TCHAR));
	_tcscpy(prog_options.app_name, argv[1]);

	prog_options.app_params = malloc((app_params_len + 1) * sizeof(_TCHAR));
	_tcscpy(prog_options.app_params, argv[2]);

	return 0;
}

_TCHAR * get_app_path()
{
	return prog_options.app_name;
}

_TCHAR * get_app_params()
{
	return prog_options.app_params;
}

void cleanup_options()
{
	if (prog_options.app_name)
	{
		free(prog_options.app_name);
		prog_options.app_name = NULL;
	}
	if (prog_options.app_params)
	{
		free(prog_options.app_params);
		prog_options.app_params = NULL;
	}
}

void print_usage(_TCHAR *p)
{
	_ftprintf(stdout, _T("Usage: %s prog.exe [params]\n"), p);
}