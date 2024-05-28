#pragma once

#include "core/runtime.h"
#include "core/csharp.h"
#include "core/math/floats.h"

private void Close(void);
private void Start(void);
private void SetTimeProvider(double(*Provider)());

struct _Application Application =
{
	.Start = Start,
	.Close = Close,
	.SetTimeProvider = SetTimeProvider,
	.FixedUpdateTimeInterval = 0,
	.InternalState = {
		.CloseApplicationFlag = 0,
		.PreviousTime = 0.0,
		.TimeProvider = null
	}
};

private void Close(void)
{
	Application.InternalState.CloseApplicationFlag = true;
}

private void Start(void)
{
	RunOnStartMethods();

	while (Application.InternalState.CloseApplicationFlag is false)
	{
		RunOnUpdateMethods();

		RunOnAfterUpdateMethods();

		if (Application.InternalState.TimeProvider)
		{
			const double time = Application.InternalState.TimeProvider();

			const double targetTime = Application.InternalState.PreviousTime + Application.FixedUpdateTimeInterval;

			if (time >= targetTime) {
				RunOnFixedUpdateMethods();
				RunOnAfterFixedUpdateMethods();

				Application.InternalState.PreviousTime = time;
			}
		}
	}

	RunOnCloseMethods();
}

private void SetTimeProvider(double(*Provider)())
{
	Application.InternalState.TimeProvider = Provider;
}