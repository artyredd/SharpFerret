#pragma once

#include "core/runtime.h"
#include "core/csharp.h"
#include "core/math/floats.h"

private void Close(void);
private void Start(void);
private void SetTimeProvider(double(*Provider)());
private void AppendEvent(RuntimeEventType, void(*Method)(void));
private void RemoveEvent(RuntimeEventType, void(*Method)(void));

struct _Application Application =
{
	.Start = Start,
	.Close = Close,
	.SetTimeProvider = SetTimeProvider,
	.FixedUpdateTimeInterval = 0,
	.AppendEvent = AppendEvent,
	.RemoveEvent = RemoveEvent,
	.InternalState = {
		.CloseApplicationFlag = 0,
		.PreviousTime = 0.0,
		.TimeProvider = null,
		.RuntimeStarted = false
	}
};

private void Close(void)
{
	Application.InternalState.CloseApplicationFlag = true;
}

DEFINE_CONTAINERS(_VoidMethod);
DEFINE_CONTAINERS(array(_VoidMethod));

array(_VoidMethod) GLOBAL_StartEvents;
array(_VoidMethod) GLOBAL_CloseEvents;
array(_VoidMethod) GLOBAL_UpdateEvents;
array(_VoidMethod) GLOBAL_AfterUpdateEvents;
array(_VoidMethod) GLOBAL_FixedUpdateEvents;
array(_VoidMethod) GLOBAL_AfterFixedUpdateEvents;
array(array(_VoidMethod)) GLOBAL_Events = empty_stack_array(array(_VoidMethod), 7);

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
	default:
		return "Unkown";
	}
#undef case_return
}

private void AppendEvent(RuntimeEventType eventType, void(*Method)(void))
{
	if (Application.InternalState.RuntimeStarted is false)
	{
		fprintf_red(stderr, "Failed to append event %s, runtime not started", EventName(eventType));
		throw(InvalidArgumentException);
	}

	if (Method is null)
	{
		fprintf_red(stderr, "Failed to append event %s, method was null", EventName(eventType));
		throw(InvalidArgumentException);
	}

	arrays(_VoidMethod).Append(at(GLOBAL_Events, eventType), Method);
}

private void RemoveEvent(RuntimeEventType eventType, void(*Method)(void))
{
	if (Application.InternalState.RuntimeStarted is false)
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
	array(_VoidMethod) eventArray = at(GLOBAL_Events, eventType);

	const int index = arrays(_VoidMethod).IndexOf(eventArray, Method);

	// remove the event
	arrays(_VoidMethod).RemoveIndex(eventArray, index);
}

private void InitializeRuntime()
{
	GLOBAL_StartEvents = dynamic_array(_VoidMethod, 0);
	GLOBAL_CloseEvents = dynamic_array(_VoidMethod, 0);
	GLOBAL_UpdateEvents = dynamic_array(_VoidMethod, 0);
	GLOBAL_AfterUpdateEvents = dynamic_array(_VoidMethod, 0);
	GLOBAL_FixedUpdateEvents = dynamic_array(_VoidMethod, 0);
	GLOBAL_AfterFixedUpdateEvents = dynamic_array(_VoidMethod, 0);

	// none
	arrays(array(_VoidMethod)).Append(GLOBAL_Events, null);

	arrays(array(_VoidMethod)).Append(GLOBAL_Events, GLOBAL_StartEvents);
	arrays(array(_VoidMethod)).Append(GLOBAL_Events, GLOBAL_CloseEvents);
	arrays(array(_VoidMethod)).Append(GLOBAL_Events, GLOBAL_UpdateEvents);
	arrays(array(_VoidMethod)).Append(GLOBAL_Events, GLOBAL_AfterUpdateEvents);
	arrays(array(_VoidMethod)).Append(GLOBAL_Events, GLOBAL_FixedUpdateEvents);
	arrays(array(_VoidMethod)).Append(GLOBAL_Events, GLOBAL_AfterFixedUpdateEvents);


	Application.InternalState.RuntimeStarted = true;
}

private void ExecuteEvents(array(_VoidMethod) events)
{
	for (int i = 0; i < events->Count; i++)
	{
		at(events, i)();
	}
}

private void Start(void)
{
	InitializeRuntime();

	RunOnStartMethods();
	ExecuteEvents(GLOBAL_StartEvents);

	while (Application.InternalState.CloseApplicationFlag is false)
	{
		RunOnUpdateMethods();
		ExecuteEvents(GLOBAL_UpdateEvents);

		RunOnAfterUpdateMethods();
		ExecuteEvents(GLOBAL_AfterUpdateEvents);

		if (Application.InternalState.TimeProvider)
		{
			const double time = Application.InternalState.TimeProvider();

			const double targetTime = Application.InternalState.PreviousTime + Application.FixedUpdateTimeInterval;

			if (time >= targetTime) {
				RunOnFixedUpdateMethods();
				ExecuteEvents(GLOBAL_FixedUpdateEvents);

				RunOnAfterFixedUpdateMethods();
				ExecuteEvents(GLOBAL_AfterFixedUpdateEvents);

				Application.InternalState.PreviousTime = time;
			}
		}
	}

	ExecuteEvents(GLOBAL_CloseEvents);
	RunOnCloseMethods();
}

private void SetTimeProvider(double(*Provider)())
{
	Application.InternalState.TimeProvider = Provider;
}