#ifndef PROG_OPTS_H
#define PROG_OPTS_H

#include <tchar.h>

int parse_options(int argc, _TCHAR *argv[]);
_TCHAR * get_app_path();
_TCHAR * get_app_params();
_TCHAR * get_user_name();
_TCHAR * get_password();

void print_usage(_TCHAR *p);
void cleanup_options();

#endif