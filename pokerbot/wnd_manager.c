#include <Windows.h>
#include "wnd_manager.h"
#include "basic_log.h"

#define WND_MNG_CLASS_NAME _T("wnd_mng_class")

static struct s_wnd_manager
{
	HWND hwnd;
	HINSTANCE h_instance;
	s_thread thread;
} wnd_manager;


DWORD WINAPI wnd_manager_proc(LPVOID param);
ATOM wnd_manager_register_class();
LRESULT CALLBACK wnd_manager_wndproc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param);
int wnd_manager_init_main_wnd(int nCmdShow);


int wnd_manager_setup()
{
	int ret;

	log_dbg(_T("Setting up windows manager\n"));
	wnd_manager.thread.proc = &wnd_manager_proc;
	wnd_manager.h_instance = GetModuleHandle(NULL);

	if (ret = create_thread(&wnd_manager.thread))
		return ret;

	log_dbg(_T("Done.\n"));

	return 0;
}
int wnd_manager_start()
{
	int ret = 0;

	/* Dispatch manager thread */
	if (ret = wnd_manager_setup())
	{
		log_error(_T("Cannot initialize window manager\n"));
		return ret;
	}

	/* Wait for it to start */
	WaitForSingleObject(wnd_manager.thread.ev_started, INFINITE);

	return 0;
}

s_thread * wnd_manager_get_thread()
{
	return &wnd_manager.thread;
}

int wnd_manager_terminate()
{
	SendMessage(wnd_manager.hwnd, WM_DESTROY, 0, 0);
	return 0;
}

HWND get_wnd_manager_hwnd()
{
	return wnd_manager.hwnd;
}

void wnd_manager_cleanup()
{
	cleanup_thread(&wnd_manager.thread);
}

DWORD WINAPI wnd_manager_proc(LPVOID param)
{
	MSG msg;

	log_dbg("Registering window manager main class\n");
	if (!wnd_manager_register_class())
	{
		log_error("Could not initialize window manager class");
		return 0;
	}

	if (!wnd_manager_init_main_wnd(SW_SHOW))
	{
		log_error(_T("Could not create main window\n"));
		return 0;
	}

	SetEvent(wnd_manager.thread.ev_started);

	log_dbg(_T("Window manager started\n"));

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
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = WND_MNG_CLASS_NAME;
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex);
}

int wnd_manager_init_main_wnd(int nCmdShow)
{

	wnd_manager.hwnd = CreateWindow(WND_MNG_CLASS_NAME, _T("main_wnd"), WS_POPUPWINDOW | WS_THICKFRAME | WS_VISIBLE,
		100, 100, 800, 300, NULL, NULL, wnd_manager.h_instance, NULL);

	if (!wnd_manager.hwnd)
	{
		return 0;
	}

	ShowWindow(wnd_manager.hwnd, nCmdShow);
	UpdateWindow(wnd_manager.hwnd);

	return 1;
}

LRESULT CALLBACK wnd_manager_wndproc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hwnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, w_param, l_param);
	}
	return 0;
}
