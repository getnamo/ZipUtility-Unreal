#pragma once

#include "WindowsFileUtilityPrivatePCH.h"
#include "WFULambdaRunnable.h"

uint64 WFULambdaRunnable::ThreadNumber = 0;

FQueuedThreadPool* WFULambdaRunnable::ThreadPool = nullptr;

WFULambdaRunnable::WFULambdaRunnable(TFunction< void()> InFunction)
{
	FunctionPointer = InFunction;
	
	FString threadStatGroup = FString::Printf(TEXT("FLambdaRunnable%d"), ThreadNumber++);
	Thread = NULL; 
	Thread = FRunnableThread::Create(this, *threadStatGroup, 0, TPri_BelowNormal); //windows default = 8mb for thread, could specify more
}

WFULambdaRunnable::~WFULambdaRunnable()
{
	if (Thread == NULL)
	{
		delete Thread;
		Thread = NULL;
	}

	ThreadPool->Destroy();
}

//Run
uint32 WFULambdaRunnable::Run()
{
	if (FunctionPointer)
		FunctionPointer();

	//UE_LOG(LogClass, Log, TEXT("FLambdaRunnable %d Run complete"), Number);
	return 0;
}

void WFULambdaRunnable::Exit()
{
	//UE_LOG(LogClass, Log, TEXT("FLambdaRunnable %d Exit"), Number);

	//delete ourselves when we're done
	delete this;
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

void WFULambdaRunnable::EnsureCompletion()
{
	Thread->WaitForCompletion();
}

WFULambdaRunnable* WFULambdaRunnable::RunLambdaOnBackGroundThread(TFunction< void()> InFunction)
{
	if (FPlatformProcess::SupportsMultithreading())
	{
		//UE_LOG(LogClass, Log, TEXT("FLambdaRunnable RunLambdaBackGroundThread"));
		return new WFULambdaRunnable(InFunction);
	}
	return nullptr;
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

FGraphEventRef WFULambdaRunnable::RunShortLambdaOnGameThread(TFunction< void()> InFunction)
{
	return FFunctionGraphTask::CreateAndDispatchWhenReady(InFunction, TStatId(), nullptr, ENamedThreads::GameThread);
}

