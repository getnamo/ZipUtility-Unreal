#include "ZipUtilityPrivatePCH.h"
#include "SevenZipCallbackHandler.h"
#include "WFULambdaRunnable.h"
#include "ZipOperation.h"

UZipOperation::UZipOperation()
{
	CallbackHandler = nullptr;
}

void UZipOperation::StopOperation()
{
	if (ThreadPoolWork != nullptr)
	{
		WFULambdaRunnable::RemoveLambdaFromQueue(ThreadPoolWork);
	}

	if (CallbackHandler != nullptr)
	{
 		CallbackHandler->bCancelOperation = true;
		CallbackHandler = nullptr;
	}
}

void UZipOperation::SetCallbackHandler(SevenZipCallbackHandler* Handler)
{
	CallbackHandler = Handler;
}

void UZipOperation::SetThreadPoolWorker(IQueuedWork* Work)
{
	ThreadPoolWork = Work;
}

