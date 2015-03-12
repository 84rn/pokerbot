#include "thread_utils.h"
#include "basic_log.h"


int create_thread(s_thread *thread_data)
{
	HANDLE h;
	DWORD suspended = (thread_data->suspended ? CREATE_SUSPENDED : 0);

	if (!thread_data)
	{
		log_error("Empty thread data\n");
		return 1;
	}

	if (h = CreateThread(NULL, 0, thread_data->proc, &thread_data, suspended, &thread_data->id))
	{
		thread_data->handle = h;
		thread_data->ev_terminate = CreateEvent(NULL, TRUE, 0, NULL);
		thread_data->ev_started = CreateEvent(NULL, TRUE, 0, NULL);
		log_dbg(_T("Thread %d created\n"), thread_data->id);

		return 0;
	}
	else
		log_error(_T("Could not create thread\n"));

	return 1;
}

int terminate_thread(HANDLE ev_terminate)
{
	return SetEvent(ev_terminate);
}

void cleanup_thread(s_thread *thread_data)
{
	CloseHandle(thread_data->ev_started);
	CloseHandle(thread_data->ev_terminate);
	CloseHandle(thread_data->handle);
}