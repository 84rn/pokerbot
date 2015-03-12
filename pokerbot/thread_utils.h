#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

#include <Windows.h>
#include <tchar.h>

typedef DWORD (WINAPI *tthread_proc)(LPVOID);

typedef struct 
{
	HANDLE handle;
	HANDLE ev_terminate;
	HANDLE ev_started;
	DWORD id;
	tthread_proc proc;
	int suspended;

} s_thread;

int create_thread(s_thread *thread_data);
int terminate_thread(HANDLE ev_terminate);
void cleanup_thread(s_thread *thread_data);


#endif