#include "thread_utils.h"
#include "basic_log.h"

int thread_create(s_thread *thread_data)
{
	HANDLE h;
	DWORD suspended = (thread_data->suspended ? CREATE_SUSPENDED : 0);

	if (!thread_data)
	{
		log_err(_T("Empty thread data"));
		return 1;
	}

	if (h = CreateThread(NULL, 0, thread_data->proc, &thread_data, suspended, &thread_data->id))
	{
		thread_data->handle = h;
		thread_data->event.terminate = CreateEvent(NULL, TRUE, 0, NULL);
		thread_data->event.started = CreateEvent(NULL, TRUE, 0, NULL);
		thread_data->event.idle = CreateEvent(NULL, TRUE, 0, NULL);
		thread_data->param = NULL;

		log_dbg(_T("Thread [%#x] created"), thread_data->id);

		return 0;
	}
	else
		log_err(_T("[%d] Could not create thread "), GetLastError());

	return 1;
}

int thread_terminate(HANDLE ev_terminate)
{
	return SetEvent(ev_terminate);
}

void thread_cleanup(s_thread *thread_data)
{
	CloseHandle(thread_data->event.started);
	CloseHandle(thread_data->event.terminate);
	CloseHandle(thread_data->event.idle);	
	CloseHandle(thread_data->handle);

	thread_data->param = NULL;
	thread_data->proc = NULL;
	thread_data->id = 0;
}

DWORD thread_wait_for_init(s_thread *thread)
{
	HANDLE ev[2] = { thread->event.idle, thread->handle };
	DWORD ret;

	ret = WaitForMultipleObjects(2, ev, FALSE, INFINITE);

	switch (ret)
	{
	case WAIT_OBJECT_0:
		return 0;
	case WAIT_OBJECT_0 + 1:
		GetExitCodeThread(thread->handle, &ret);
		return ret;
	case WAIT_FAILED:
		log_dbg(_T("Waiting for init thread [%d] failed"), GetLastError());
		return GetLastError();
	default:
		break;
	}

	return ret;
}