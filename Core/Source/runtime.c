#pragma once

#include "core/runtime.h"
#include "core/csharp.h"
#include "core/math/floats.h"
#include "core/tasks.h"
#include "core/os.h"
#include "core/atomic.h"

private void Close(void);
private void Start(void);
private void SetTimeProvider(double(*Provider)());
private void AppendEvent(RuntimeEventType, void(*Method)(void));
private void RemoveEvent(RuntimeEventType, void(*Method)(void));
private void SetParentApplication(struct _Application* parent);
private array(array(_VoidMethod)) GlobalEvents();
private void InitializeThreadedEvents();

struct _Application Application =
{
	.Start = Start,
	.Close = Close,
	.SetTimeProvider = SetTimeProvider,
	.FixedUpdateTimeInterval = 0,
	.AppendEvent = AppendEvent,
	.RemoveEvent = RemoveEvent,
	.SetParentApplication = SetParentApplication,
	.InitializeThreadedEvents = InitializeThreadedEvents,
	.InternalState = {
		.CloseApplicationFlag = 0,
		.PreviousTime = 0.0,
		.TimeProvider = null,
		.RuntimeStarted = false,
		.StartMethodsBegan = false,
		.ParentProcessApplication = null,
		.GlobalEvents = GlobalEvents
	}
};

#define ApplicationOrParent (Application.InternalState.ParentProcessApplication ? Application.InternalState.ParentProcessApplication : &Application )

private void SetParentApplication(struct _Application* parent)
{
	Application.InternalState.ParentProcessApplication = parent;
}

private void Close(void)
{
	ApplicationOrParent->InternalState.CloseApplicationFlag = true;
}

array(_VoidMethod) GLOBAL_StartEvents;
array(_VoidMethod) GLOBAL_CloseEvents;
array(_VoidMethod) GLOBAL_UpdateEvents;
array(_VoidMethod) GLOBAL_AfterUpdateEvents;
array(_VoidMethod) GLOBAL_FixedUpdateEvents;
array(_VoidMethod) GLOBAL_AfterFixedUpdateEvents;
array(_VoidMethod) GLOBAL_RenderEvents;
array(_VoidMethod) GLOBAL_AfterRenderEvents;

array(array(_VoidMethod)) GLOBAL_Events = empty_stack_array(array(_VoidMethod), sizeof(RuntimeEventTypes) / sizeof(RuntimeEventType));

array(_VoidMethod) GLOBAL_EventStack;
array(Task) GLOBAL_Workers;
locker GLOBAL_WorkerLock;
bool GLOBAL_NewWorkSignal = false;
long GLOBAL_WorkersWorkingCount = 0;

private array(array(_VoidMethod)) GlobalEvents()
{
	return GLOBAL_Events;
}

private const char* EventName(RuntimeEventType eventType)
{
#define case_return(number,word) case number: return #word
	switch (eventType)
	{
		case_return(0, RuntimeEventTypes.None);
		case_return(1, RuntimeEventTypes.Start);
		case_return(2, RuntimeEventTypes.Close);
		case_return(3, RuntimeEventTypes.Update);
		case_return(4, RuntimeEventTypes.AfterUpdate);
		case_return(5, RuntimeEventTypes.FixedUpdate);
		case_return(6, RuntimeEventTypes.AfterFixedUpdate);
		case_return(7, RuntimeEventTypes.Render);
		case_return(8, RuntimeEventTypes.AfterRender);
	default:
		return "Unkown";
	}
#undef case_return
}

private void AppendEvent(RuntimeEventType eventType, _VoidMethod Method)
{
	if (ApplicationOrParent->InternalState.RuntimeStarted is false)
	{
		fprintf_red(stderr, "Failed to append event %s, runtime not started", EventName(eventType));
		throw(InvalidArgumentException);
	}

	if (Method is null)
	{
		fprintf_red(stderr, "Failed to append event %s, method was null", EventName(eventType));
		throw(InvalidArgumentException);
	}

	array(_VoidMethod) array = at(ApplicationOrParent->InternalState.GlobalEvents(), eventType);

	if (arrays(_VoidMethod).IndexOf(array, Method) is - 1)
	{
		arrays(_VoidMethod).Append(array, Method);
	}
	else
	{
		fprintf_red(stderr, "Can't add the same method twice\n%s", "");
		throw(InvalidLogicException);
	}

	// if we already executing the application on starts
	// then we're hooking a plugin thats hooked after the start
	// of the program and we should execute their on start
	// immediately
	if (eventType is RuntimeEventTypes.Start and ApplicationOrParent->InternalState.StartMethodsBegan)
	{
		Method();
	}
}

private void RemoveEvent(RuntimeEventType eventType, void(*Method)(void))
{
	if (ApplicationOrParent->InternalState.RuntimeStarted is false)
	{
		fprintf_red(stderr, "Failed to remove event %s, runtime not started", EventName(eventType));
		throw(InvalidArgumentException);
	}

	// nothing to remove
	if (Method is null or eventType is RuntimeEventTypes.None)
	{
		return;
	}

	// find the index of the method
	array(_VoidMethod) eventArray = at(ApplicationOrParent->InternalState.GlobalEvents(), eventType);

	const int index = arrays(_VoidMethod).IndexOf(eventArray, Method);

	// remove the event
	arrays(_VoidMethod).RemoveIndex(eventArray, index);
}

// this is the work that all the runtime threads should do
private int WorkerJob(void* state)
{
	ignore_unused(state);

	// wait for new work
	Task task = Tasks.CurrentTask();

	// exit when asked
	while (task->RequestExitFlag is false)
	{
		while (GLOBAL_EventStack->Count)
		{
			_VoidMethod method = null;



			lock(GLOBAL_WorkerLock,
				if (GLOBAL_EventStack->Count)
				{
					method = arrays(_VoidMethod).Pop(GLOBAL_EventStack);
					_InterlockedIncrement(&GLOBAL_WorkersWorkingCount);

					/*fprintf_yellow(stdout, "Got work, workers active: %i Total Tasks:%lli [%i]\n",
						GLOBAL_WorkersWorkingCount,
						GLOBAL_EventStack->Count,
						Tasks.ThreadId());*/

				}
			);

			if (method)
			{
				method();

				/*fprintf_yellow(stdout, "Finshed work, workers active: %i Total Tasks:%lli [%i]\n",
					GLOBAL_WorkersWorkingCount,
					GLOBAL_EventStack->Count,
					Tasks.ThreadId());*/


				_InterlockedDecrement(&GLOBAL_WorkersWorkingCount);
				Tasks.NotifyAllThreadsAddressChanged(&GLOBAL_WorkersWorkingCount);
			}

		}

		Tasks.WaitOnAddress(&GLOBAL_NewWorkSignal, sizeof(bool), Tasks.Forever);
	}

	return 0;
}

private void InitializeWorkers(array(Task) workers)
{
	for (int i = 0; i < OperatingSystem.ThreadCount() - 1; i++)
	{
		arrays(Task).Append(workers, Tasks.Run(Tasks.Create(WorkerJob), null));
	}
}

private void ExpandMultiThreadedEvents(RuntimeEventType type, _VoidMethod* methods)
{
	int i = 0;
	while (methods[i] != null)
	{
		_VoidMethod method = methods[i];

		AppendEvent(type, method);

		i++;
	}
}

private void InitializeThreadedEvents()
{
	// append multi threaded events individually
	ExpandMultiThreadedEvents(RuntimeEventTypes.Update, GetUpdateMethods());
	ExpandMultiThreadedEvents(RuntimeEventTypes.AfterUpdate, GetAfterUpdateMethods());
	ExpandMultiThreadedEvents(RuntimeEventTypes.FixedUpdate, GetFixedUpdateMethods());
	ExpandMultiThreadedEvents(RuntimeEventTypes.AfterFixedUpdate, GetAfterFixedUpdateMethods());
}

private void InitializeRuntime()
{
	GLOBAL_StartEvents = dynamic_array(_VoidMethod, 0);
	GLOBAL_CloseEvents = dynamic_array(_VoidMethod, 0);
	GLOBAL_UpdateEvents = dynamic_array(_VoidMethod, 0);
	GLOBAL_AfterUpdateEvents = dynamic_array(_VoidMethod, 0);
	GLOBAL_FixedUpdateEvents = dynamic_array(_VoidMethod, 0);
	GLOBAL_AfterFixedUpdateEvents = dynamic_array(_VoidMethod, 0);
	GLOBAL_RenderEvents = dynamic_array(_VoidMethod, 0);
	GLOBAL_AfterRenderEvents = dynamic_array(_VoidMethod, 0);

	// none
	arrays(array(_VoidMethod)).Append(GLOBAL_Events, null);

	arrays(array(_VoidMethod)).Append(GLOBAL_Events, GLOBAL_StartEvents);
	arrays(array(_VoidMethod)).Append(GLOBAL_Events, GLOBAL_CloseEvents);
	arrays(array(_VoidMethod)).Append(GLOBAL_Events, GLOBAL_UpdateEvents);
	arrays(array(_VoidMethod)).Append(GLOBAL_Events, GLOBAL_AfterUpdateEvents);
	arrays(array(_VoidMethod)).Append(GLOBAL_Events, GLOBAL_FixedUpdateEvents);
	arrays(array(_VoidMethod)).Append(GLOBAL_Events, GLOBAL_AfterFixedUpdateEvents);
	arrays(array(_VoidMethod)).Append(GLOBAL_Events, GLOBAL_RenderEvents);
	arrays(array(_VoidMethod)).Append(GLOBAL_Events, GLOBAL_AfterRenderEvents);


	GLOBAL_EventStack = dynamic_array(_VoidMethod, 0);
	GLOBAL_Workers = dynamic_array(Task, OperatingSystem.ThreadCount() - 1);

	InitializeWorkers(GLOBAL_Workers);

	Application.InternalState.RuntimeStarted = true;

	InitializeThreadedEvents();
}

private void ExecuteEvents(array(_VoidMethod) events, bool mainthread)
{
	for (int i = 0; i < events->Count; i++)
	{
		if (mainthread)
		{
			at(events, i)();
		}
		else
		{
			lock(GLOBAL_WorkerLock,
				arrays(_VoidMethod).Append(GLOBAL_EventStack, at(events, i));
			//fprintf(stdout, "Adding work: Active Workers:%i, Total Work:%lli [%i]\n", GLOBAL_WorkersWorkingCount, GLOBAL_EventStack->Count, Tasks.ThreadId());
			);

			Tasks.NotifyAddressChanged(&GLOBAL_NewWorkSignal);
		}
	}

	// wait for workers to finish
	if (mainthread is false && GLOBAL_WorkersWorkingCount)
	{
		while (GLOBAL_WorkersWorkingCount > 0)
		{
			_sleep(1);
		}
		//fprintf_red(stdout, "All events finished continuing to next events [%i]\n", Tasks.ThreadId());

	}
}

private void Start(void)
{
	if (Application.InternalState.ParentProcessApplication)
	{
		fprintf_red(stderr, "Attempted to start runtime of child plugin. Plugins automatically hook their runtimes. %s\n", "");
		throw(InvalidLogicException);
	}

	InitializeRuntime();

	RunOnStartMethods();
	ExecuteEvents(GLOBAL_StartEvents, true);
	Application.InternalState.StartMethodsBegan = true;

	while (Application.InternalState.CloseApplicationFlag is false)
	{
		ExecuteEvents(GLOBAL_UpdateEvents, false);

		ExecuteEvents(GLOBAL_AfterUpdateEvents, false);

		if (Application.InternalState.TimeProvider)
		{
			const double time = Application.InternalState.TimeProvider();

			const double targetTime = Application.InternalState.PreviousTime + Application.FixedUpdateTimeInterval;

			if (time >= targetTime) {
				ExecuteEvents(GLOBAL_FixedUpdateEvents, false);
				ExecuteEvents(GLOBAL_AfterFixedUpdateEvents, false);

				Application.InternalState.PreviousTime = time;
			}
		}

		RunOnRenderMethods();
		ExecuteEvents(GLOBAL_RenderEvents, true);

		RunOnAfterRenderMethods();
		ExecuteEvents(GLOBAL_AfterRenderEvents, true);
	}

	ExecuteEvents(GLOBAL_CloseEvents, true);
	RunOnCloseMethods();
}

private void SetTimeProvider(double(*Provider)())
{
	Application.InternalState.TimeProvider = Provider;
}