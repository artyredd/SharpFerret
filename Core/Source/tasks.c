#include "core/csharp.h"

#include "core/tasks.h"
#include "core/memory.h"

#ifdef WIN32
#include <Windows.h>
#include <synchapi.h>
#endif // WIN32

#define MAX_HANDLES 1024

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
private Task CurrentTask();
private Task TaskForThread(int threadId);

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
	.CurrentTask = CurrentTask,
	.TaskForThread = TaskForThread,
	.Dispose = Dispose
};

DEFINE_TYPE_ID(Task);

private Task CreateTask(int (*Method)(void* state))
{
	REGISTER_TYPE(Task);

	Task task = Memory.Alloc(sizeof(struct _task), TaskTypeId);

	task->Method = Method;
	task->Status = TaskStatus.Created;

	return task;
}

private void Dispose(Task task)
{
	Stop(task->ThreadHandle);

	Memory.Free(task, TaskTypeId);
}


#define MAX_THREAD_ASSIGNMENTS 1024
Task GLOBAL_ThreadAssignments[MAX_THREAD_ASSIGNMENTS];

private int MethodWrapper(Task task)
{
	int threadAssignment = task->ThreadId % MAX_THREAD_ASSIGNMENTS;
	GLOBAL_ThreadAssignments[threadAssignment] = task;

	task->Status = TaskStatus.Running;
	NotifyAllThreadsAddressChanged(&task->Status);

	task->Method(task->InternalState.StatePointer);

	task->Status = TaskStatus.RanToCompletion;
	NotifyAllThreadsAddressChanged(&task->Status);

	task->ThreadHandle = null;
	task->ThreadId = 0;
	task->InternalState.StatePointer = null;

	// close our own handle
	if (!Stop(task)) {
		task->Status = TaskStatus.Faulted;
		NotifyAllThreadsAddressChanged(&task->Status);
	};

	GLOBAL_ThreadAssignments[threadAssignment] = 0;

	return task->Status isnt TaskStatus.RanToCompletion;
}

private void RunAll(array(Task) tasks, array(void_ptr) states)
{
	for (int i = 0; i < tasks->Count; i++)
	{
		void* state = states ? (states->Count ? at(states, i) : null) : null;
		Run(at(tasks, i), state);
	}
}

private Task TaskForThread(int threadId)
{
	return GLOBAL_ThreadAssignments[threadId % MAX_THREAD_ASSIGNMENTS];
}

private Task CurrentTask()
{
	return TaskForThread(ThreadId());
}

private Task Run(Task task, void* state)
{
	if (task->Status isnt TaskStatus.Created)
	{
		// task already being started
		return task;
	}

	task->Status = TaskStatus.WaitingForActivation;
	NotifyAllThreadsAddressChanged(&task->Status);

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
	void* handles[MAX_HANDLES];

	if (tasks->Count >= MAX_HANDLES)
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
	void* handles[MAX_HANDLES];

	if (tasks->Count >= MAX_HANDLES)
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
	while (task->Status != state)
	{
		if (_WaitOnAddress(&task->Status, sizeof(byte), milliseconds) is false)
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