#include <Windows.h>
#include "wnd_manager.h"
#include "basic_log.h"
#include "main_bot.h"

#define WND_MNG_CLASS_NAME _T("wnd_mng_class")
#define IDC_LOG_FIELD 101
#define LOG_FIELD_H 300
#define LOG_FIELD_W 500

static struct s_wnd_manager
{
	WNDPROC org_log_wnd_proc;

	HWND hwnd;
	HWND log_window;
	HINSTANCE h_instance;
	s_thread thread;
} wnd_manager;


DWORD WINAPI wnd_manager_proc(LPVOID param);
ATOM wnd_manager_register_class();
LRESULT CALLBACK wnd_manager_wndproc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param);
LRESULT CALLBACK wnd_manager_log_wndproc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param);
int wnd_manager_create_main_wnd(int nCmdShow);


int wnd_manager_setup()
{
	int ret;

	log_dbg(_T("Setting up windows manager"));
	wnd_manager.thread.proc = &wnd_manager_proc;
	wnd_manager.h_instance = GetModuleHandle(NULL);

	if (ret = thread_create(&wnd_manager.thread))
		return ret;

	log_dbg(_T("Done."));

	return 0;
}
int wnd_manager_start()
{
	int ret = 0;

	/* Dispatch manager thread */
	if (ret = wnd_manager_setup())
	{
		log_err(_T("Cannot initialize window manager"));
		return ret;
	}

	/* Wait for started or fail*/
	if (ret = thread_wait_for_init(&wnd_manager.thread))
	{
		log_err(_T("[%d] Window manager thread init failed"), ret);
		return ret;
	}

	return 0;
}

s_thread * wnd_manager_get_thread()
{
	return &wnd_manager.thread;
}

int wnd_manager_terminate()
{
	PostMessage(wnd_manager.hwnd, WM_CLOSE, 0, 0);
	return 0;
}

HWND wnd_manager_get_hwnd()
{
	return wnd_manager.hwnd;
}

void wnd_manager_cleanup()
{
	thread_cleanup(&wnd_manager.thread);
	wnd_manager.hwnd = NULL;
	wnd_manager.h_instance = NULL;
}

void wnd_manager_log_mbot(_TCHAR *log)
{
	_TCHAR *t_log = calloc(_tcslen(log) + 2 + 2 + 1, sizeof(_TCHAR));
	_tcscat(t_log, "> ");
	_tcscat(t_log, log);
	_tcscat(t_log, "\r\n");

	SendMessage(wnd_manager.log_window, EM_REPLACESEL, 0, (LPARAM)t_log);

	free(t_log);
}

DWORD WINAPI wnd_manager_proc(LPVOID param)
{
	MSG msg;
	int ret;

	SetEvent(wnd_manager.thread.event.started);

	log_dbg(_T("Registering window manager main class"));
	if (!wnd_manager_register_class())
	{
		log_err(_T("Could not initialize window manager class"));
		return GetLastError();
	}

	if (ret = wnd_manager_create_main_wnd(SW_SHOW))
	{
		log_err(_T("Could not create main window"));
		return ret;
	}

	log_dbg(_T("Window manager started"));

	SetEvent(wnd_manager.thread.event.idle);

	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	log_dbg(_T("Window manager closing..."));

	return 0;
}

ATOM wnd_manager_register_class()
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = wnd_manager_wndproc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = wnd_manager.h_instance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = WND_MNG_CLASS_NAME;
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex);
}

int wnd_manager_create_main_wnd(int nCmdShow)
{
	int w, h;
	RECT work_area = { 0 };
	SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0);

	w = work_area.right;
	h = work_area.bottom;

	/* Put it in the bottom right corner */
	wnd_manager.hwnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, WND_MNG_CLASS_NAME, _T("main_wnd"), WS_POPUP,
		w - LOG_FIELD_W, h - LOG_FIELD_H, LOG_FIELD_W, LOG_FIELD_H, NULL, NULL, wnd_manager.h_instance, NULL);

	if (!wnd_manager.hwnd)
	{
		return GetLastError();
	}

	ShowWindow(wnd_manager.hwnd, nCmdShow);
	UpdateWindow(wnd_manager.hwnd);

	return 0;
}

HWND wnd_manager_create_log_edit(HWND hwnd)
{
	HWND h = CreateWindowEx(0,
		_T("EDIT"),
		"",
		WS_CHILD | WS_VISIBLE | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL,
		5,			// 5 px for margins
		5,
		LOG_FIELD_W - 10,
		LOG_FIELD_H - 10,
		hwnd,
		(HMENU)IDC_LOG_FIELD, wnd_manager.h_instance, NULL);

	if (!h)
	{
		log_err(_T("[%d] Cannot create log field"), GetLastError());
		return 0;
	}

	wnd_manager.org_log_wnd_proc = (WNDPROC)SetWindowLongPtr(h,
		GWLP_WNDPROC, (LONG_PTR)wnd_manager_log_wndproc);

	SetClassLongPtr(h, GCLP_HCURSOR, (LONG_PTR)NULL);	

	return h;
}

LRESULT CALLBACK wnd_manager_log_wndproc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
	switch (message)
	{
	case WM_NCHITTEST:
		return HTTRANSPARENT;
	default:
		return wnd_manager.org_log_wnd_proc(hwnd, message, w_param, l_param);
	}

	return 0;
}

LRESULT CALLBACK wnd_manager_wndproc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
	PAINTSTRUCT ps;
	HDC hdc;
	UINT hit_test;
	HWND handle, *p_handle;

	int ret;

	switch (message)
	{
	case WM_CREATE:
		wnd_manager.log_window = wnd_manager_create_log_edit(hwnd);
		break;
	case WM_CTLCOLORSTATIC:
		hdc = (HDC)w_param;
		SetTextColor(hdc, RGB(215, 40, 92)); // set text color
		SetBkColor(hdc, RGB(0, 0, 0));
		return (LRESULT)GetStockObject(BLACK_BRUSH);		
	case WM_NCHITTEST:
		hit_test = DefWindowProc(hwnd, WM_NCHITTEST, w_param, l_param);
	
		if (hit_test == HTCLIENT)
			return HTCAPTION;
		else
			return hit_test;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hwnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;				
	case WM_LOG_MAIN_BOT:
		wnd_manager_log_mbot((_TCHAR *)l_param);
		break;
	case WM_GET_PROCESS_CAPTURE:		
		 ret = AttachThreadInput(GetCurrentThreadId(), main_bot_get_app()->p_info.dwThreadId, TRUE);
		 handle = GetCapture();		
		 p_handle = (HWND*)w_param;
		 *p_handle = handle;
		 ret = AttachThreadInput(GetCurrentThreadId(), main_bot_get_app()->p_info.dwThreadId, FALSE);
		 break;
	case WM_GET_PROCESS_ACTIVE_WND:
		ret = AttachThreadInput(GetCurrentThreadId(), main_bot_get_app()->p_info.dwThreadId, TRUE);
		handle = GetActiveWindow();
		p_handle = (HWND*)w_param;
		*p_handle = handle;
		ret = AttachThreadInput(GetCurrentThreadId(), main_bot_get_app()->p_info.dwThreadId, FALSE);
		break;		
	default:
		return DefWindowProc(hwnd, message, w_param, l_param);
	}

	return 0;
}
