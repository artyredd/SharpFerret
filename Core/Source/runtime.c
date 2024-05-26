#pragma once

#include "core/runtime.h"
#include "core/csharp.h"

private void Close(void);
private void Start(void);

struct _Application Application =
{
	.Start = Start,
	.Close = Close,
	.InternalState = {
		._CloseApplicationFlag = 0
	}
};

private void Close(void)
{
	Application.InternalState._CloseApplicationFlag = true;
}

private void Start(void)
{
	RunOnStartMethods();

	while (Application.InternalState._CloseApplicationFlag is false)
	{
		RunOnUpdateMethods();

		RunOnAfterUpdateMethods();
	}

	RunOnCloseMethods();
}