#ifndef SEND_INPUT_H
#define SEND_INPUT_H

#include <Windows.h>
#include <tchar.h>

void send_click(HWND hwnd, int x, int y);
void send_ctrl_char(HWND hwnd, _TCHAR c);
void send_vkey(HWND hwnd, WORD key);
void send_char(HWND hwnd, _TCHAR c);
void send_text(HWND hwnd, _TCHAR *t, int ms);

#endif