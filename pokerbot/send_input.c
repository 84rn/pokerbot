#include <ctype.h>
#include "send_input.h"

void send_click(HWND hwnd, int x, int y)
{
	SendMessage(hwnd, WM_LBUTTONDOWN, 0, MAKELPARAM(x, y));
	SendMessage(hwnd, WM_LBUTTONUP, 0, MAKELPARAM(x, y));
}


void send_ctrl_char(HWND hwnd, _TCHAR c)
{
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0;
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	// Press the "Ctrl" key
	ip.ki.wVk = VK_CONTROL;
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));

	// Press the key
	ip.ki.wVk = c;
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));

	// Release the key
	ip.ki.wVk = c;
	ip.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &ip, sizeof(INPUT));

	// Release the "Ctrl" key
	ip.ki.wVk = VK_CONTROL;
	ip.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &ip, sizeof(INPUT));

}


void send_vkey(HWND hwnd, WORD key)
{
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0;
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	// Press the key
	ip.ki.wVk = key;
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));

	// Release the key
	ip.ki.wVk = key;
	ip.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &ip, sizeof(INPUT));
}

void send_char(HWND hwnd, _TCHAR c)
{
	int caps = (GetKeyState(VK_CAPITAL) & 0x0001);
	INPUT ip;
	WORD key = VkKeyScanEx(c, GetKeyboardLayout(0));
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0;
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	if (caps)
	{
		// Press the CAPS key
		ip.ki.wVk = VK_CAPITAL;
		ip.ki.dwFlags = 0; // 0 for key press
		SendInput(1, &ip, sizeof(INPUT));

		ip.ki.wVk = VK_CAPITAL;
		ip.ki.dwFlags = KEYEVENTF_KEYUP; 
		SendInput(1, &ip, sizeof(INPUT));
	}

	if (_istupper(c))
	{
		// Press the SHIFT key
		ip.ki.wVk = VK_SHIFT;
		ip.ki.dwFlags = 0; // 0 for key press
		SendInput(1, &ip, sizeof(INPUT));
	}

	// Press the key
	ip.ki.wVk = key;
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));

	// Release the key
	ip.ki.wVk = key;
	ip.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &ip, sizeof(INPUT));

	if (_istupper(c))
	{
		ip.ki.wVk = VK_SHIFT;
		ip.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, &ip, sizeof(INPUT));
	}
	
	if (caps)
	{
		ip.ki.wVk = VK_CAPITAL;
		ip.ki.dwFlags = 0; // 0 for key press
		SendInput(1, &ip, sizeof(INPUT));

		ip.ki.wVk = VK_CAPITAL;
		ip.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, &ip, sizeof(INPUT));
	}
}

void send_text(HWND hwnd, _TCHAR *t, int ms)
{
	while (*t)
	{
		send_char(hwnd, *t++);
		Sleep(ms);
	}
}