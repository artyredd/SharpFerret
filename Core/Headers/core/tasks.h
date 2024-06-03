#pragma once

#include "core/csharp.h"
#include "core/array.h"

typedef byte TaskState;

static struct _taskState {
	TaskState Created;
	TaskState WaitingForActivation;
	TaskState WaitingToRun;
	TaskState Running;
	TaskState WaitingForChildrenToComplete;
	TaskState RanToCompletion;
	TaskState Canceled;
	TaskState Faulted;
} TaskStatus = {
	.Created = 0,
	.WaitingForActivation = 1,
	.WaitingToRun = 2,
	.Running = 3,
	.WaitingForChildrenToComplete = 4,
	.RanToCompletion = 5,
	.Canceled = 6,
	.Faulted = 7
};

typedef ulong WaitStatus;

static struct _waitStatus
{
	WaitStatus Abandoned;
	WaitStatus Signaled;
	WaitStatus Timedout;
	WaitStatus Failed;
} WaitStatuses = {
	.Abandoned = 0x00000080L,
	.Signaled = 0x00000000L,
	.Timedout = 0x00000102L,
	.Failed = 0xFFFFFFFF
};

struct _task {
	void* ThreadHandle;
	int ThreadId;
	int (*Method)(void* state);
	TaskState Status;
	// Tells the task to finish it's work and exit quietly
	// use Tasks.Stop() to force close a thread running a task
	bool RequestExitFlag;
	struct
	{
		void* StatePointer;
	} InternalState;
};

typedef struct _task* Task;

DEFINE_CONTAINERS(Task);

extern struct _taskMethods {
	ulong Forever;
	Task(*Create)(int(*Method)(void* state));
	Task(*Run)(Task task, void* state);
	void (*RunAll)(array(Task), array(void_ptr) states);
	bool (*Stop)(Task);
	void (*NotifyAllThreadsAddressChanged)(void* address);
	void (*NotifyAddressChanged)(void* address);
	bool (*WaitOnAddress)(volatile void* address, ulong addressSize, ulong milliseconds);
	WaitStatus(*Wait)(Task task, ulong milliseconds);
	WaitStatus(*WaitAll)(array(Task) tasks, ulong milliseconds);
	WaitStatus(*WaitAny)(array(Task) tasks, ulong milliseconds);
	int (*ThreadId)();
	// Returns the task that the current thread is executing
	Task(*CurrentTask)();
	// Returns the task that's executing on the given thread id
	Task(*TaskForThread)(int threadId);
	WaitStatus(*WaitForState)(Task task, WaitStatus state, ulong milliseconds);
	void (*Dispose)(Task);
} Tasks;