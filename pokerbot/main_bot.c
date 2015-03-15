#include <Windows.h>
#include "main_bot.h"
#include "wnd_manager.h"
#include "prog_opts.h"
#include "process_utils.h"
#include "thread_utils.h"
#include "basic_log.h"
#include "send_input.h"

static struct s_main_bot
{
	s_thread thread;
	process_data app;
	HWND selected_window;
} main_bot;

DWORD WINAPI main_bot_proc(LPVOID param);
int main_bot_start_app();
BOOL main_bot_find_wnd_title(_TCHAR *title);
int main_bot_find_wnd_title_multi(_TCHAR **titles);
int main_bot_login(_TCHAR *u, _TCHAR *p);

void log_mbot(_TCHAR *log);

int main_bot_setup()
{
	int ret;

	log_dbg(_T("Setting up main bot"));
	main_bot.thread.proc = main_bot_proc;

	if (ret = thread_create(&main_bot.thread))
		return ret;

	log_dbg(_T("Done."));

	return 0;
}

int main_bot_start()
{
	int ret;

	/* Dispatch main bot thread */
	if (ret = main_bot_setup())
	{
		log_err(_T("Cannot setup main bot"));
		return ret;
	}

	/* Wait for started or fail*/
	if (ret = thread_wait_for_init(&main_bot.thread))
	{
		log_err(_T("[%d] Main bot thread init failed"), ret);
		return ret;
	}

	return 0;
}

DWORD WINAPI main_bot_proc(LPVOID param)
{
	int terminate = 0;
	DWORD wait, ret;

	SetEvent(main_bot.thread.event.started);

	/* Start window manager */
	if (ret = wnd_manager_start())
		return ret;

	/* Finished init */
	SetEvent(main_bot.thread.event.idle);

	/* Say hello */
	log_mbot("Hello, this is Chappy!\r\n");

	/* Dispatch main process thread */
	if (ret = main_bot_start_app())
		return ret;

	log_mbot("Done.");

	/* Try to log in*/
	if (ret = main_bot_login(get_user_name(), get_password()))
	{
		log_mbot("Login failed!");
		return ret;
	}

	log_mbot("Login succesful!");

	wait = WaitForSingleObject(wnd_manager_get_thread()->handle, INFINITE);

	log_mbot("Bye!");

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
	return SetEvent(main_bot.thread.event.terminate);
}

process_data * main_bot_get_app()
{
	return &main_bot.app;
}

void main_bot_cleanup()
{
	thread_cleanup(&main_bot.thread);
	main_bot.selected_window = NULL;
}

void log_mbot(_TCHAR *log)
{
	SendMessage(wnd_manager_get_hwnd(), WM_LOG_MAIN_BOT, (WPARAM)NULL, (LPARAM)log);
}

int main_bot_start_app()
{
	int ret;

	log_mbot("Waking up main process...");
	ret = process_start_app(get_app_path(), get_app_params(), &main_bot.app);

	if (!ret)
	{
		return WaitForInputIdle(main_bot.app.p_info.hProcess, INFINITE);
	}
	else
	{
		log_err(_T("[%d] Could not wake up main process"), GetLastError());
		return 1;
	}
}
BOOL CALLBACK main_bot_enum_children(HWND hwnd, LPARAM lParam)
{
	_TCHAR t[500];
	_TCHAR s[500];

	GetClassName(hwnd, t, 500);
	GetWindowText(hwnd, s, 500);
	log_msg(_T("Class: %s | Name: %s"), t, s);
	return TRUE;
}

int main_bot_login(_TCHAR *u, _TCHAR *p)
{
	int ret = 0;
	int capture_retry = 0;
	HWND h_process = NULL;

	log_mbot(_T("Trying to log in..."));
	while (main_bot_find_wnd_title(_T("Account")));
	log_mbot(_T("Login window found"));

	/* Attach to process to find capture window */
	h_process = NULL;
	capture_retry = 0;
	while (!h_process &&  capture_retry++ < 10)
	{
		SendMessage(wnd_manager_get_hwnd(), WM_GET_PROCESS_CAPTURE, (WPARAM)&h_process, 0);
		Sleep(100);
	}

	if (h_process)
	{
		log_mbot("Killing the tricky capture window");

		/* Send a click to unblock window*/
		send_click(h_process, 10, 10);
	}

	/* Send input for login*/
	h_process = FindWindowEx(main_bot.selected_window, NULL, "CefBrowserWindow", NULL);
	if (h_process)
	{
		/* Find the embedded browser */
		h_process = FindWindowEx(h_process, NULL, "WebViewHost", NULL);

		if (h_process)
			log_mbot("Got the login browser window");
		else
			ret = 1;
	}
	else
		ret = 1;

	if (ret)
	{
		log_mbot("Couldn't find the login child window");
		return 1;
	}

	send_click(h_process, 109, 65);
	send_ctrl_char(h_process, 'A');
	send_vkey(h_process, VK_DELETE);

	/* Enter username */
	send_text(h_process, u, 10);

	Sleep(500);
	send_click(h_process, 65, 100);
	send_ctrl_char(h_process, 'A');
	send_vkey(h_process, VK_DELETE);

	/* Enter password */
	send_text(h_process, p, 10);

	/* Push the button! */
	Sleep(100);
	send_click(h_process, 100, 140); 

	/* Kill the popups */
	log_mbot(_T("Killing post login windows"));
	_TCHAR *windows[] = { "EE-RENTRY", "postLogin", 0 };

	while (main_bot_find_wnd_title_multi(windows));
	SendMessage(main_bot.selected_window, WM_CLOSE, 0, 0);

	/* Attach to process to find capture window */
	h_process = NULL;
	capture_retry = 0;
	while (!h_process && capture_retry++ < 10)
	{
		SendMessage(wnd_manager_get_hwnd(), WM_GET_PROCESS_CAPTURE, (WPARAM)&h_process, 0);
		Sleep(100);
	}

	if (h_process)
	{
		log_mbot("Focusing the hall");

		/* Send a click to unblock window*/
		send_click(h_process, 10, 10);
	}

	return 0;
}


BOOL CALLBACK get_process_wnd_title_callback(HWND hwnd, LPARAM lParam)
{
	_TCHAR *title = (_TCHAR *)lParam;

	DWORD org_pid = main_bot.app.p_info.dwProcessId;
	DWORD pid;

	GetWindowThreadProcessId(hwnd, &pid);

	if (pid == org_pid)
	{
		_TCHAR t[500];
		GetWindowText(hwnd, t, 500);

		if (!_tcsncmp(title, t, 5))
		{
			main_bot.selected_window = hwnd;
			log_dbg(_T("Found window: %s [%p]"), t, hwnd);
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CALLBACK get_process_wnd_title_multi_callback(HWND hwnd, LPARAM lParam)
{
	DWORD org_pid = main_bot.app.p_info.dwProcessId;
	DWORD pid;

	GetWindowThreadProcessId(hwnd, &pid);

	if (pid == org_pid)
	{
		_TCHAR t[500];
		_TCHAR **temp = (_TCHAR **)lParam;
		GetWindowText(hwnd, t, 500);

		while (*temp)
		{
			if (!_tcsncmp(*temp++, t, 5))
			{
				main_bot.selected_window = hwnd;
				log_dbg(_T("Found window: %s [%p]"), t, hwnd);
				return FALSE;
			}
		}
	}

	return TRUE;
}

int main_bot_find_wnd_title(_TCHAR *title)
{
	return EnumWindows(get_process_wnd_title_callback, (LPARAM)title);
}

int main_bot_find_wnd_title_multi(_TCHAR **titles)
{
	return EnumWindows(get_process_wnd_title_multi_callback, (LPARAM)titles);
}

