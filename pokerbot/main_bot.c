#include <Windows.h>
#include <CommCtrl.h>
#include <string.h>
#include "main_bot.h"
#include "wnd_manager.h"
#include "prog_opts.h"
#include "process_utils.h"
#include "thread_utils.h"
#include "basic_log.h"
#include "send_input.h"
#include "menu_func.h"
#include "wnd_func.h"

#define M_SELECT(x, y) m_select_menu_path(main_bot.main_menu, (x), (y))

static struct s_main_bot
{
	s_thread thread;
	process_data app;
	HWND selected_window;
	menu_node_t *main_menu;
} main_bot;

DWORD WINAPI main_bot_proc(LPVOID param);
int main_bot_start_app();
int main_bot_select_wnd_by_title(_TCHAR *title);
int main_bot_select_wnd_by_title_multi(_TCHAR **titles);
int main_bot_select_child_by_title(HWND parent, _TCHAR *title);
int main_bot_select_child_by_title_multi(HWND parent, _TCHAR **titles);
int main_bot_build_menu_tree();

int main_bot_select_child_by_class(HWND parent, _TCHAR *);

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
	log_mbot("Hello, this is Chappie!\r\n");

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

	if (ret = main_bot_build_menu_tree())
	{
		log_dbg(_T("Could not build menu tree"));
		return 1;
	}	

	wait = WaitForSingleObject(wnd_manager_get_thread()->handle, INFINITE);

	//log_mbot("Bye!");

	//wnd_manager_terminate();
	//WaitForSingleObject(wnd_manager_get_thread()->handle, INFINITE);

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
	m_destroy_node(main_bot.main_menu);
	main_bot.main_menu = NULL;
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

int main_bot_login(_TCHAR *u, _TCHAR *p)
{
	int ret = 0;
	int retry = 0;
	HWND h_process = NULL;

	log_mbot(_T("Trying to log in..."));
	for (; main_bot_select_wnd_by_title(_T("Account")); Sleep(50));
	log_mbot(_T("Login window found"));

	/* Attach to process to find capture window */
	for (h_process = NULL, retry = 0; !h_process && retry++ < 10; Sleep(100))
		SendMessage(wnd_manager_get_hwnd(), WM_GET_PROCESS_CAPTURE, (WPARAM)&h_process, 0);

	if (h_process)
	{
		log_mbot("Killing the tricky capture window");

		/* Send a click to unblock window*/
		send_click(h_process, 10, 10);
	}

	for (; main_bot_select_child_by_class(main_bot.selected_window, _T("WebViewHost")); Sleep(50));
	log_mbot("Got the login browser window");

	h_process = main_bot.selected_window;
	Sleep(500);
	send_click(h_process, 110, 65);
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

	/* If not logged in, pass or user are wrong, quit */
	for (retry = 0; !main_bot_select_wnd_by_title(_T("Account")) && retry++ < 100; Sleep(100));
	if (retry > 100)
	{
		log_mbot(_T("Wrong user/pass. Quitting."));
		return 1;
	}

	/* Kill the popups */
	log_mbot(_T("Killing post login windows"));
	_TCHAR *windows[] = { "EE-RENTRY", "postLogin", 0 };

	for (; main_bot_select_wnd_by_title_multi(windows); Sleep(50));

	SendMessage(main_bot.selected_window, WM_CLOSE, 0, 0);

	/* Attach to process to find capture window */
	for (h_process = NULL, retry = 0; !h_process && retry++ < 10; Sleep(100))
		SendMessage(wnd_manager_get_hwnd(), WM_GET_PROCESS_CAPTURE, (WPARAM)&h_process, 0);

	if (h_process)
	{
		log_mbot("Focusing the hall");

		/* Send a click to unblock window*/
		send_click(h_process, 10, 10);
	}

	return 0;
}

BOOL CALLBACK get_process_wnd_by_class_callback(HWND hwnd, LPARAM lParam)
{
	_TCHAR *class = (_TCHAR *)lParam;

	DWORD org_pid = main_bot.app.p_info.dwProcessId;
	DWORD pid;

	GetWindowThreadProcessId(hwnd, &pid);

	if (pid == org_pid)
	{
		_TCHAR t[500];
		GetClassName(hwnd, t, 500);

		if (!_tcsncmp(class, t, 5))
		{
			main_bot.selected_window = hwnd;
			log_dbg(_T("Found window class: %s [%p]"), t, hwnd);
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CALLBACK get_app_wnd_by_title_callback(HWND hwnd, LPARAM lParam)
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
			log_dbg(_T("Found window title: %s [%p]"), t, hwnd);
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CALLBACK get_app_wnd_by_title_multi_callback(HWND hwnd, LPARAM lParam)
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
				log_dbg(_T("Found first window title: %s [%p]"), t, hwnd);
				return FALSE;
			}
		}
	}

	return TRUE;
}

int main_bot_select_wnd_by_title(_TCHAR *title)
{
	return EnumWindows(get_app_wnd_by_title_callback, (LPARAM)title);
}

int main_bot_select_wnd_by_title_multi(_TCHAR **titles)
{
	return EnumWindows(get_app_wnd_by_title_multi_callback, (LPARAM)titles);
}

int main_bot_select_child_by_title(HWND parent, _TCHAR *title)
{
	return EnumChildWindows(parent, get_app_wnd_by_title_callback, (LPARAM)title);
}

int main_bot_select_child_by_title_multi(HWND parent, _TCHAR **titles)
{
	return EnumChildWindows(parent, get_app_wnd_by_title_multi_callback, (LPARAM)titles);
}

int main_bot_select_child_by_class(HWND parent, _TCHAR *class)
{
	return EnumChildWindows(parent, get_process_wnd_by_class_callback, (LPARAM)class);
}

int main_bot_build_menu_tree()
{
	/* Get Main menu*/
	HWND lobby, seated, child;
	menu_node_t *node;
	menu_t *menu = NULL;

	main_bot_select_wnd_by_title(_T("bwin.com"));
	if (!main_bot.selected_window)
	{
		log_err(_T("Could not find lobby window"));
		return 1;
	}

	lobby = main_bot.selected_window;

	main_bot.selected_window = wnd_get_child_no(lobby, 6);
	if (!main_bot.selected_window)
	{
		log_err(_T("Could not find main menu window"));
		return 1;
	}

	/* Main Menu */
	menu = menu_build(main_bot.selected_window, TREEVIEW, 0, FALSE);
	if (!menu)
	{
		log_err(_T("Could not build main menu"));
		return 1;
	}

	main_bot.main_menu = m_new_node(NID_MAIN_MENU, menu);			

	main_bot_select_child_by_title(lobby, "Seated");
	if (!main_bot.selected_window)
	{
		log_err(_T("Could not find Seated Players window"));
		return 1;
	}

	seated = main_bot.selected_window;
	
	main_bot_select_child_by_title(main_bot.selected_window, "Casino");

	/* Poker Gametypes */
	menu = menu_build(main_bot.selected_window, BUTTON, 6, FALSE);
	node = m_new_node(NID_POKER, menu);
	m_add_node_to_ID(main_bot.main_menu, NID_MAIN_MENU, node);

	/* Set all the meta nodes */
	node = m_new_node(NID_POKER_CASINO, NULL);
	m_add_node_to_ID(main_bot.main_menu, NID_POKER, node);
	node = m_new_node(NID_POKER_PLAYMONEY, NULL);
	m_add_node_to_ID(main_bot.main_menu, NID_POKER, node);
	node = m_new_node(NID_POKER_TOURNAMENTS, NULL);
	m_add_node_to_ID(main_bot.main_menu, NID_POKER, node);
	node = m_new_node(NID_POKER_SITNGOS, NULL);
	m_add_node_to_ID(main_bot.main_menu, NID_POKER, node);
	node = m_new_node(NID_POKER_FASTFORWARD, NULL);
	m_add_node_to_ID(main_bot.main_menu, NID_POKER, node);
	node = m_new_node(NID_POKER_CASH, NULL);
	m_add_node_to_ID(main_bot.main_menu, NID_POKER, node);
	
	/* Playmoney*/
	child = wnd_get_child_no(seated, 46);

	main_bot.selected_window = wnd_get_child_no(child, 1);
	menu = menu_build(main_bot.selected_window, LISTVIEW, 0, FALSE); 
	node = m_new_node(NID_PPLAYMONEY_GAMENAME, menu);
	m_add_node_to_ID(main_bot.main_menu, NID_POKER_PLAYMONEY, node);

	main_bot.selected_window = wnd_get_child_no(child, 2);
	menu = menu_build(main_bot.selected_window, LISTVIEW, 0, TRUE);
	node = m_new_node(NID_PPLAYMONEY_STAKES, menu);
	m_add_node_to_ID(main_bot.main_menu, NID_POKER_PLAYMONEY, node);

	main_bot.selected_window = wnd_get_child_no(child, 3);
	menu = menu_build(main_bot.selected_window, LISTVIEW, 0, TRUE);
	node = m_new_node(NID_PPLAYMONEY_SEATS, menu);
	m_add_node_to_ID(main_bot.main_menu, NID_POKER_PLAYMONEY, node);

	/* Tables */
	main_bot.selected_window = wnd_get_child_no(seated, 9);
	menu = menu_build(main_bot.selected_window, LISTVIEW, 0, TRUE);
	node = m_new_node(NID_PPLAYMONEY_TABLES, menu);
	m_add_node_to_ID(main_bot.main_menu, NID_POKER_PLAYMONEY, node);

	/* TEST */
	M_SELECT(NID_PPLAYMONEY_GAMENAME, IX_PPPT_OMAHA);
	M_SELECT(NID_PPLAYMONEY_GAMENAME, IX_PPNL_HOLDEM);
	M_SELECT(NID_PPLAYMONEY_STAKES, 0);
	M_SELECT(NID_PPLAYMONEY_SEATS, 0);
	M_SELECT(NID_PPLAYMONEY_TABLES, 15);
	return 0;
}