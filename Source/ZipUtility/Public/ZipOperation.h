#pragma once
#include "Object.h"
#include "ZipOperation.generated.h"

class SevenZipCallbackHandler;
/**
 * Used to track a zip/unzip operation on the WFULambdaRunnable ThreadPool and allows the ability to terminate the
 * operation early.
 */
UCLASS(BlueprintType)
class ZIPUTILITY_API UZipOperation : public UObject
{
	GENERATED_BODY()
public:
	UZipOperation();

	// Stops this zip/unzip if it is still valid. This stop event can occur in two places:
	// 1. The ThreadPool queue, if it can be stopped here then the operation has not yet started.
	// 2. The SevenZip layer, once the operation has started, it can be canceled while still running.
	// Note that calling this carries no guarantees of a successful stop, the end result might be one of:
	// * The file still got extracted (you were too late)
	// * The file was never extracted (caught it on time)
	// * The file was created but is of zero size (oops)
	// * The file was created, is of non-zero size but is not all there (cut off in the middle)
	// So, it could be a good idea to do some file housekeeping afterward.
	UFUNCTION(BlueprintCallable, Category = "Zip Operation")
	void StopOperation();

	// Set the callback handler
	void SetCallbackHandler(SevenZipCallbackHandler* Handler);

	// Set the queued work
	void SetThreadPoolWorker(IQueuedWork* Work);
	
private:
	// A pointer to the callback for this operation. Once the operation completes, this
	// pointer will become invalid.
	SevenZipCallbackHandler* CallbackHandler;

	// The work that was queued on the async threadpool in WFULambdaRunnable
	IQueuedWork* ThreadPoolWork;
};
