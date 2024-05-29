#include "core/csharp.h"

#include "core/tasks.h"
#include "core/memory.h"

#ifdef WIN32
#include <Windows.h>
#include <synchapi.h>
#endif // WIN32

private Task CreateTask(int (*Method)(void* state));
private Task Run(Task task, void* state);
private bool Stop(Task task);
private void NotifyAllThreadsAddressChanged(void* address);
private void NotifyAddressChanged(void* address);
private bool _WaitOnAddress(volatile void* address, ulong addressSize, ulong milliseconds);
private int ThreadId();
private WaitState Wait(Task task, ulong milliseconds);
private WaitState WaitForState(Task task, WaitState state, ulong milliseconds);
private void RunAll(array(Task) tasks, array(void_ptr) states);
private WaitState WaitAll(array(Task) tasks, ulong milliseconds);
private WaitState WaitAny(array(Task) tasks, ulong milliseconds);
private void Dispose(Task);

struct _taskMethods Tasks = {
	.Create = CreateTask,
	.Run = Run,
	.Stop = Stop,
	.ThreadId = ThreadId,
	.WaitOnAddress = _WaitOnAddress,
	.NotifyAddressChanged = NotifyAddressChanged,
	.NotifyAllThreadsAddressChanged = NotifyAllThreadsAddressChanged,
	.Wait = Wait,
	.WaitForState = WaitForState,
	.RunAll = RunAll,
	.WaitAll = WaitAll,
	.WaitAny = WaitAny,
	.Dispose = Dispose
};

DEFINE_TYPE_ID(Task);

private Task CreateTask(int (*Method)(void* state))
{
	REGISTER_TYPE(Task);

	Task task = Memory.Alloc(sizeof(struct _task), TaskTypeId);

	task->Method = Method;
	task->State = TaskStatus.Created;

	return task;
}

private void Dispose(Task task)
{
	Stop(task->ThreadHandle);

	Memory.Free(task, TaskTypeId);
}

private int MethodWrapper(Task task)
{
	task->State = TaskStatus.Running;
	NotifyAllThreadsAddressChanged(&task->State);

	task->Method(task->InternalState.StatePointer);

	task->State = TaskStatus.RanToCompletion;
	NotifyAllThreadsAddressChanged(&task->State);

	task->ThreadHandle = null;
	task->ThreadId = 0;
	task->InternalState.StatePointer = null;

	// close our own handle
	if (!Stop(task)) {
		task->State = TaskStatus.Faulted;
		NotifyAllThreadsAddressChanged(&task->State);
	};

	return task->State isnt TaskStatus.RanToCompletion;
}

private void RunAll(array(Task) tasks, array(void_ptr) states)
{
	for (int i = 0; i < tasks->Count; i++)
	{
		void* state = states ? (states->Count ? at(states, i) : null) : null;
		Run(at(tasks, i), state);
	}
}

private Task Run(Task task, void* state)
{
	if (task->State isnt TaskStatus.Created)
	{
		// task already being started
		return task;
	}

	task->State = TaskStatus.WaitingForActivation;
	NotifyAllThreadsAddressChanged(&task->State);

	task->InternalState.StatePointer = state;

	task->ThreadHandle = CreateThread(
		NULL,                   // Default security attributes
		0,                      // Default stack size
		MethodWrapper,         // Thread function
		task,            // Parameter to pass to the thread function
		0,                      // Default creation flags
		&task->ThreadId             // Receive thread identifier
	);

	return task;
}

private int ThreadId()
{
	return GetCurrentThreadId();
}

private bool Stop(Task task)
{
	if (task->ThreadHandle)
	{
		return CloseHandle(task->ThreadHandle);
	}

	return false;
}

private WaitState WaitAny(array(Task) tasks, ulong milliseconds)
{
	void* handles[2048];

	if (tasks->Count >= 2048)
	{
		throw(OutOfMemoryException);
	}

	for (int i = 0; i < tasks->Count; i++)
	{
		handles[i] = at(tasks, i);
	}

	return WaitForMultipleObjects(tasks->Count, handles, false, milliseconds);
}

private WaitState WaitAll(array(Task) tasks, ulong milliseconds)
{
	void* handles[2048];

	if (tasks->Count >= 2048)
	{
		throw(OutOfMemoryException);
	}

	for (int i = 0; i < tasks->Count; i++)
	{
		handles[i] = at(tasks, i);
	}

	return WaitForMultipleObjects(tasks->Count, handles, true, milliseconds);
}

private WaitState Wait(Task task, ulong milliseconds)
{
	return WaitForSingleObject(task->ThreadHandle, milliseconds);
}

private WaitState WaitForState(Task task, WaitState state, ulong milliseconds)
{
	while (task->State != state)
	{
		if (_WaitOnAddress(&task->State, sizeof(byte), milliseconds) is false)
		{
			return WaitStatus.Timedout;
		}
	}

	return WaitStatus.Signaled;
}

// A thread can use the WaitOnAddress function to wait for the value of a target address to change from some undesired value to any other value. This enables threads to wait for a value to change without having to spin
private bool _WaitOnAddress(volatile void* address, ulong addressSize, ulong milliseconds)
{
	return WaitOnAddress(address, ((byte*)address + 0), addressSize, milliseconds);
}

private void NotifyAddressChanged(void* address)
{
	WakeByAddressSingle(address);
}

private void NotifyAllThreadsAddressChanged(void* address)
{
	WakeByAddressAll(address);
}