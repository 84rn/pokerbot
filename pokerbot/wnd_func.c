#include "wnd_func.h"


BOOL CALLBACK wnd_first_child(HWND hwnd, LPARAM lParam);

HWND wnd_get_sibling_no(HWND root, int n)
{
	HWND next = root;

	if (n < 0)
		return 0;

	while (n && (next = GetNextWindow(next, GW_HWNDNEXT)))
		--n;

	if (n > 0)
		return 0;
	
	return next;
}

HWND wnd_get_child_no(HWND root, int n)
{
	HWND child;
	EnumChildWindows(root, wnd_first_child, (LPARAM)&child);
	return wnd_get_sibling_no(child, n - 1);
}

BOOL CALLBACK wnd_first_child(HWND hwnd, LPARAM lParam)
{
	HWND *h = (HWND *)lParam;
	*h = hwnd;
	return FALSE;
}