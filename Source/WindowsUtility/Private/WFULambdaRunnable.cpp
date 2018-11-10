#pragma once

#include "WFULambdaRunnable.h"
#include "WindowsFileUtilityPrivatePCH.h"

uint64 WFULambdaRunnable::ThreadNumber = 0;

FQueuedThreadPool* WFULambdaRunnable::ThreadPool = nullptr;

WFULambdaRunnable::~WFULambdaRunnable()
{
	ThreadPool->Destroy();
}

TFuture<void> WFULambdaRunnable::RunLambdaOnBackGroundThread(TFunction< void()> InFunction)
{
	return Async(EAsyncExecution::Thread, InFunction);
}

TFuture<void> WFULambdaRunnable::RunLambdaOnBackGroundThreadPool(TFunction< void()> InFunction)
{
	return Async(EAsyncExecution::ThreadPool, InFunction);
}

FGraphEventRef WFULambdaRunnable::RunShortLambdaOnGameThread(TFunction< void()> InFunction)
{
	return FFunctionGraphTask::CreateAndDispatchWhenReady(InFunction, TStatId(), nullptr, ENamedThreads::GameThread);
}

void WFULambdaRunnable::InitThreadPool(int32 NumberOfThreads)
{
	if (ThreadPool == nullptr)
	{
		ThreadPool = FQueuedThreadPool::Allocate();
		int32 NumThreadsInThreadPool = NumberOfThreads;
		ThreadPool->Create(NumThreadsInThreadPool, 32 * 1024);
	}
}


IQueuedWork* WFULambdaRunnable::AddLambdaToQueue(TFunction< void()> InFunction)
{
	if (ThreadPool == nullptr)
	{
		WFULambdaRunnable::InitThreadPool(FPlatformMisc::NumberOfIOWorkerThreadsToSpawn());
	}

	if (ThreadPool)
	{
		return AsyncLambdaPool(*ThreadPool, InFunction);
	}
	return nullptr;
}

bool WFULambdaRunnable::RemoveLambdaFromQueue(IQueuedWork* Work)
{
	if (ThreadPool)
	{
		return ThreadPool->RetractQueuedWork(Work);
	}
	return false;
}

