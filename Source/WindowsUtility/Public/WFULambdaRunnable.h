// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

/*
Long duration lambda wrapper, which are generally not supported by the taskgraph system. New thread per lambda and they will auto-delete upon
completion.
*/
class WINDOWSFILEUTILITY_API WFULambdaRunnable : public FRunnable
{
private:
	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;

	// Used to give each thread a unique stat group
	static uint64 ThreadNumber;

	//Lambda function pointer
	TFunction< void()> FunctionPointer;

	// A queued threadpool used to run lambda's in the background. This has lazy initialization and is meant to be used with
	// - AddLambdaToQueue
	// - RemoveLAmbdaFromQueue	
	static FQueuedThreadPool* ThreadPool;
public:
	//Constructor / Destructor
	WFULambdaRunnable(TFunction< void()> InFunction);
	virtual ~WFULambdaRunnable();

	// Begin FRunnable interface.
	virtual uint32 Run() override;
	virtual void Exit() override;
	// End FRunnable interface

	// Initializes the queued thread pool. This is called lazily when the first task is added to the queue
	// but can also be called by hand to initialize with a specific number of threads. The default number
	// of threads is FPlatformMisc::NumberOfIOWorkerThreadsToSpawn() which last I checked was hard-coded
	// at 4. <NOTE> that if you want to call this by hand, you need to do so before ever calling AddLambdaToQueue.
	static void InitThreadPool(int32 NumberOfThreads);

	/** Makes sure this thread has stopped properly */
	void EnsureCompletion();

	// 	Runs the passed lambda on the background thread, new thread per call
	static WFULambdaRunnable* RunLambdaOnBackGroundThread(TFunction< void()> InFunction);

	// Adds a lambda to be ran on the queued thread pool. Returns a pointer to IQueuedWork which
	// can be used to later remove the queued job from the pool assuming it hasn't been processed.
	static IQueuedWork* AddLambdaToQueue(TFunction< void()> InFunction);

	// Removes a lambda from the thread queue
	static bool RemoveLambdaFromQueue(IQueuedWork* Work);

	// Runs a short lambda on the game thread via task graph system	
	static FGraphEventRef RunShortLambdaOnGameThread(TFunction< void()> InFunction);

private:
	// This was yanked from Engine/Source/Runtime/Core/Public/Async/Async.h (originally called AsyncPool(..)). FQueuedThreadPool doesn't have 
	// much documentation, so using the engine code as reference, pretty much everyone seems to use this templated function to queue up work.
	// It was modified to return an IQueuedWork instead of a TFuture to be more convenient for actually removing items from the queue.
	template<typename ResultType>
	static IQueuedWork* AsyncLambdaPool(FQueuedThreadPool& ThreadPool, TFunction<ResultType()> Function, TFunction<void()> CompletionCallback = TFunction<void()>())
	{
		TPromise<ResultType> Promise(MoveTemp(CompletionCallback));
		TFuture<ResultType> Future = Promise.GetFuture();
		IQueuedWork* Work = new TAsyncQueuedWork<ResultType>(MoveTemp(Function), MoveTemp(Promise));
		ThreadPool.AddQueuedWork(Work);
		return Work;
	}
};

