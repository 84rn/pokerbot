#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

#include <Windows.h>
#include <tchar.h>

typedef DWORD(WINAPI *tthread_proc)(LPVOID);

typedef struct
{
	HANDLE started;
	HANDLE idle;
	HANDLE terminate;
} thread_events;

typedef struct
{
	HANDLE handle;
	DWORD id;
	thread_events event;
	tthread_proc proc;
	int suspended;
	void *param;
} s_thread;

int thread_create(s_thread *thread_data);
int thread_terminate(HANDLE ev_terminate);
void thread_cleanup(s_thread *thread_data);
DWORD thread_wait_for_init(s_thread *thread);

#endif