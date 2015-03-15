#include <stdlib.h>
#include <stdio.h>
#include "prog_opts.h"
#include "basic_log.h"

static struct s_prog_options
{
	_TCHAR *app_name;
	_TCHAR *app_params;
	_TCHAR *user;
	_TCHAR *pass;
} prog_options;

int parse_options(int argc, _TCHAR *argv[])
{
	int app_name_len, app_params_len;
	_TCHAR c;
	_TCHAR **org_argv = NULL;

	org_argv = argv;

	if (argc < 7)
	{
		print_usage(argv[0]);
		return 1;
	}

	while (--argc && (*++argv)[0] == '-')
	{
		switch (c = *++argv[0])
		{
		case 'p':			
			--argc; ++argv;
				log_dbg(_T("Setting switch -%s to %s"), *(argv - 1), *argv);
				prog_options.pass = malloc((_tcslen(*argv) + 1) * sizeof(_TCHAR));
				_tcscpy(prog_options.pass, *argv);
			
			break;
		case 'u':		
			--argc; ++argv;
				log_dbg(_T("Setting switch -%s to %s"), *(argv - 1), *argv);
				prog_options.user = malloc((_tcslen(*argv) + 1) * sizeof(_TCHAR));
				_tcscpy(prog_options.user, *argv);
			
			break;
		default:
			print_usage((org_argv)[0]);
			return 1;
		}
	}

	if (argc != 2)
	{
		print_usage(org_argv[0]);
		return 1;
	}

	app_name_len = _tcslen(org_argv[5]);
	app_params_len = _tcslen(org_argv[6]);

	if (!app_name_len)
	{
		log_err(_T("app process name empty"));
		return 1;
	}

	prog_options.app_name = malloc((app_name_len + 1) * sizeof(_TCHAR));
	_tcscpy(prog_options.app_name, org_argv[5]);

	prog_options.app_params = malloc((app_params_len + 1) * sizeof(_TCHAR));
	_tcscpy(prog_options.app_params, org_argv[6]);

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

_TCHAR * get_user_name()
{
	return prog_options.user;
}
_TCHAR * get_password()
{
	return prog_options.pass;
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
	if (prog_options.user)
	{
		free(prog_options.user);
		prog_options.user = NULL;
	}
	if (prog_options.pass)
	{
		free(prog_options.pass);
		prog_options.pass = NULL;
	}
}

void print_usage(_TCHAR *p)
{
	_ftprintf(stdout, _T("Usage: %s -u user -p password prog.exe params\n"), p);
}