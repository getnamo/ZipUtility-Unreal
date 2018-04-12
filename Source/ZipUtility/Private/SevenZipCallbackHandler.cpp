#include "ZipUtilityPrivatePCH.h"
#include "ZipFileFunctionLibrary.h"
#include "SevenZipCallbackHandler.h"

void SevenZipCallbackHandler::OnProgress(const TString& archivePath, uint64 bytes)
{
	const UObject* interfaceDelegate = ProgressDelegate;
	const uint64 bytesConst = bytes;
	const FString pathConst = FString(archivePath.c_str());

	if (bytes > 0) {
		const float ProgressPercentage = ((double)((TotalBytes)-(BytesLeft - bytes)) / (double)TotalBytes) * 100;

		UZipFileFunctionLibrary::RunLambdaOnGameThread([interfaceDelegate, pathConst, ProgressPercentage, bytesConst] 
		{
			//UE_LOG(LogClass, Log, TEXT("Progress: %d bytes"), progress);
			((IZipUtilityInterface*)interfaceDelegate)->Execute_OnProgress((UObject*)interfaceDelegate, pathConst, ProgressPercentage, bytesConst);
		});
	}
}

void SevenZipCallbackHandler::OnDone(const TString& archivePath)
{
	const UObject* interfaceDelegate = ProgressDelegate;
	const FString pathConst = FString(archivePath.c_str());

	UZipFileFunctionLibrary::RunLambdaOnGameThread([pathConst, interfaceDelegate] 
	{
		//UE_LOG(LogClass, Log, TEXT("All Done!"));
		((IZipUtilityInterface*)interfaceDelegate)->Execute_OnDone((UObject*)interfaceDelegate, pathConst, EZipUtilityCompletionState::SUCCESS);
	});
}

void SevenZipCallbackHandler::OnFileDone(const TString& archivePath, const TString& filePath, uint64 bytes)
{
	const UObject* interfaceDelegate = ProgressDelegate;
	const FString pathConst = FString(archivePath.c_str());
	const FString filePathConst = FString(filePath.c_str());
	const uint64 bytesConst = bytes;

	UZipFileFunctionLibrary::RunLambdaOnGameThread([interfaceDelegate, pathConst, filePathConst, bytesConst]
	{
		//UE_LOG(LogClass, Log, TEXT("File Done: %s, %d bytes"), filePathConst.c_str(), bytesConst);
		((IZipUtilityInterface*)interfaceDelegate)->Execute_OnFileDone((UObject*)interfaceDelegate, pathConst, filePathConst);
	});

	//Handle byte decrementing
	if (bytes > 0) {
		BytesLeft -= bytes;
		const float ProgressPercentage = ((double)(TotalBytes - BytesLeft) / (double)TotalBytes) * 100;

		UZipFileFunctionLibrary::RunLambdaOnGameThread([interfaceDelegate, pathConst, ProgressPercentage, bytes]
		{
			//UE_LOG(LogClass, Log, TEXT("Progress: %d bytes"), progress);
			((IZipUtilityInterface*)interfaceDelegate)->Execute_OnProgress((UObject*)interfaceDelegate, pathConst, ProgressPercentage, bytes);
		});
	}
}
void SevenZipCallbackHandler::OnStartWithTotal(const TString& archivePath, unsigned __int64 totalBytes)
{
	TotalBytes = totalBytes;
	BytesLeft = TotalBytes;

	const UObject* interfaceDelegate = ProgressDelegate;
	const uint64 bytesConst = TotalBytes;
	const FString pathConst = FString(archivePath.c_str());

	UZipFileFunctionLibrary::RunLambdaOnGameThread([interfaceDelegate, pathConst, bytesConst] 
	{
		//UE_LOG(LogClass, Log, TEXT("Starting with %d bytes"), bytesConst);
		((IZipUtilityInterface*)interfaceDelegate)->Execute_OnStartProcess((UObject*)interfaceDelegate, pathConst, bytesConst);
	});
}
void SevenZipCallbackHandler::OnFileFound(const TString& archivePath, const TString& filePath, int size)
{
	const UObject* interfaceDelegate = ProgressDelegate;
	const uint64 bytesConst = TotalBytes;
	const FString pathString = FString(archivePath.c_str());
	const FString fileString = FString(filePath.c_str());

	UZipFileFunctionLibrary::RunLambdaOnGameThread([interfaceDelegate, pathString, fileString, bytesConst] 
	{
		((IZipUtilityInterface*)interfaceDelegate)->Execute_OnFileFound((UObject*)interfaceDelegate, pathString, fileString, bytesConst);
	});
}
void SevenZipCallbackHandler::OnListingDone(const TString& archivePath)
{
	const UObject* interfaceDelegate = ProgressDelegate;
	const FString pathString = FString(archivePath.c_str());

	UZipFileFunctionLibrary::RunLambdaOnGameThread([interfaceDelegate, pathString] 
	{
		((IZipUtilityInterface*)interfaceDelegate)->Execute_OnDone((UObject*)interfaceDelegate, pathString, EZipUtilityCompletionState::SUCCESS);
	});
}

bool SevenZipCallbackHandler::OnCheckBreak()
{
	return bCancelOperation;
}
