#ifndef BASIC_LOG_H
#define BASIC_LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <Windows.h>
#include <tchar.h>

void log_msg(_TCHAR *l, ...);
void log_err(_TCHAR *l, ...);
void log_dbg(_TCHAR *l, ...);
int log_init();
void log_cleanup();

#endif