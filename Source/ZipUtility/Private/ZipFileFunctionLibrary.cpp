#include "ZipUtilityPrivatePCH.h"

#include "ZipFileFunctionLibrary.h"
#include "ZipFileFunctionInternalCallback.h"
#include "ListCallback.h"
#include "ProgressCallback.h"
#include "IPluginManager.h"
#include "WFULambdaRunnable.h"
#include "ZULambdaDelegate.h"
#include "SevenZipCallbackHandler.h"
#include "WindowsFileUtilityFunctionLibrary.h"

#include "7zpp.h"

using namespace SevenZip;

//Private Namespace
namespace{

	//Threaded Lambda convenience wrappers - Task graph is only suitable for short duration lambdas, but doesn't incur thread overhead
	FGraphEventRef RunLambdaOnAnyThread(TFunction< void()> InFunction)
	{
		return FFunctionGraphTask::CreateAndDispatchWhenReady(InFunction, TStatId(), nullptr, ENamedThreads::AnyThread);
	}

	//Uses proper threading, for any task that may run longer than about 2 seconds.
	void RunLongLambdaOnAnyThread(TFunction< void()> InFunction)
	{
		WFULambdaRunnable::RunLambdaOnBackGroundThread(InFunction);
	}

	// Run the lambda on the queued threadpool
	IQueuedWork* RunLambdaOnThreadPool(TFunction< void()> InFunction)
	{
		return WFULambdaRunnable::AddLambdaToQueue(InFunction);
	}

	//Private static vars
	SevenZipLibrary SZLib;

	//Utility functions
	FString PluginRootFolder()
	{
		return IPluginManager::Get().FindPlugin("ZipUtility")->GetBaseDir();
		//return FPaths::ConvertRelativePathToFull(FPaths::GameDir());
	}

	FString DLLPath()
	{
#if _WIN64

		FString PlatformString = FString(TEXT("Win64"));
#else
		FString PlatformString = FString(TEXT("Win32"));
#endif
		//Swap these to change which license you wish to fall under for zip-utility

		FString DLLString = FString("7z.dll");		//Using 7z.dll: GNU LGPL + unRAR restriction
		//FString dllString = FString("7za.dll");	//Using 7za.dll: GNU LGPL license, crucially doesn't support .zip out of the box

		return FPaths::ConvertRelativePathToFull(FPaths::Combine(*PluginRootFolder(), TEXT("ThirdParty/7zpp/dll"), *PlatformString, *DLLString));
	}

	FString ReversePathSlashes(FString forwardPath)
	{
		return forwardPath.Replace(TEXT("/"), TEXT("\\"));
	}

	bool IsValidDirectory(FString& Directory, FString& FileName, const FString& Path)
	{
		bool Found = Path.Split(TEXT("/"), &Directory, &FileName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		//try a back split
		if (!Found)
		{
			Found = Path.Split(TEXT("\\"), &Directory, &FileName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		}

		//No valid Directory found
		if (!Found)
			return false;
		else
			return true;
	}

	SevenZip::CompressionLevelEnum libZipLevelFromUELevel(ZipUtilityCompressionLevel ueLevel) {
		switch (ueLevel)
		{
		case COMPRESSION_LEVEL_NONE:
			return SevenZip::CompressionLevel::None;
		case COMPRESSION_LEVEL_FAST:
			return SevenZip::CompressionLevel::Fast;
		case COMPRESSION_LEVEL_NORMAL:
			return SevenZip::CompressionLevel::Normal;
		default:
			return SevenZip::CompressionLevel::None;
		}
	}

	SevenZip::CompressionFormatEnum libZipFormatFromUEFormat(ZipUtilityCompressionFormat UeFormat) {
		switch (UeFormat)
		{
		case COMPRESSION_FORMAT_UNKNOWN:
			return CompressionFormat::Unknown;
		case COMPRESSION_FORMAT_SEVEN_ZIP:
			return CompressionFormat::SevenZip;
		case COMPRESSION_FORMAT_ZIP:
			return CompressionFormat::Zip;
		case COMPRESSION_FORMAT_GZIP:
			return CompressionFormat::GZip;
		case COMPRESSION_FORMAT_BZIP2:
			return CompressionFormat::BZip2;
		case COMPRESSION_FORMAT_RAR:
			return CompressionFormat::Rar;
		case COMPRESSION_FORMAT_TAR:
			return CompressionFormat::Tar;
		case COMPRESSION_FORMAT_ISO:
			return CompressionFormat::Iso;
		case COMPRESSION_FORMAT_CAB:
			return CompressionFormat::Cab;
		case COMPRESSION_FORMAT_LZMA:
			return CompressionFormat::Lzma;
		case COMPRESSION_FORMAT_LZMA86:
			return CompressionFormat::Lzma86;
		default:
			return CompressionFormat::Unknown;
		}
	}

	FString defaultExtensionFromUEFormat(ZipUtilityCompressionFormat ueFormat) 
	{
		switch (ueFormat)
		{
		case COMPRESSION_FORMAT_UNKNOWN:
			return FString(TEXT(".dat"));
		case COMPRESSION_FORMAT_SEVEN_ZIP:
			return FString(TEXT(".7z"));
		case COMPRESSION_FORMAT_ZIP:
			return FString(TEXT(".zip"));
		case COMPRESSION_FORMAT_GZIP:
			return FString(TEXT(".gz"));
		case COMPRESSION_FORMAT_BZIP2:
			return FString(TEXT(".bz2"));
		case COMPRESSION_FORMAT_RAR:
			return FString(TEXT(".rar"));
		case COMPRESSION_FORMAT_TAR:
			return FString(TEXT(".tar"));
		case COMPRESSION_FORMAT_ISO:
			return FString(TEXT(".iso"));
		case COMPRESSION_FORMAT_CAB:
			return FString(TEXT(".cab"));
		case COMPRESSION_FORMAT_LZMA:
			return FString(TEXT(".lzma"));
		case COMPRESSION_FORMAT_LZMA86:
			return FString(TEXT(".lzma86"));
		default:
			return FString(TEXT(".dat"));
		}
	}

	using namespace std;

	

	//Background Thread convenience functions
	UZipOperation* UnzipFilesOnBGThreadWithFormat(const TArray<int32> FileIndices, const FString& ArchivePath, const FString& DestinationDirectory, const UObject* ProgressDelegate, ZipUtilityCompressionFormat Format)
	{
		UZipOperation* ZipOperation = NewObject<UZipOperation>();

		IQueuedWork* Work = RunLambdaOnThreadPool([ProgressDelegate, FileIndices, ArchivePath, DestinationDirectory, Format, ZipOperation] 
		{
			SevenZipCallbackHandler PrivateCallback;
			PrivateCallback.ProgressDelegate = (UObject*)ProgressDelegate;
			ZipOperation->SetCallbackHandler(&PrivateCallback);

			//UE_LOG(LogClass, Log, TEXT("path is: %s"), *path);
			SevenZipExtractor Extractor(SZLib, *ArchivePath);


			if (Format == COMPRESSION_FORMAT_UNKNOWN)
			{
				if (!Extractor.DetectCompressionFormat())
				{
					UE_LOG(LogTemp, Log, TEXT("auto-compression detection did not succeed, passing in unknown format to 7zip library."));
				}
			}
			else
			{
				Extractor.SetCompressionFormat(libZipFormatFromUEFormat(Format));
			}

			// Extract indices
			const int32 NumberFiles = FileIndices.Num(); 
			unsigned int* Indices = new unsigned int[NumberFiles]; 

			for (int32 idx = 0; idx < NumberFiles; idx++)
			{
				Indices[idx] = FileIndices[idx]; 
			}
			
			// Perform the extraction
			Extractor.ExtractFilesFromArchive(Indices, NumberFiles, *DestinationDirectory, &PrivateCallback);

			// Clean up the indices
			delete Indices;

			// Null out the callback handler now that we're exiting
			ZipOperation->SetCallbackHandler(nullptr);
		});

		ZipOperation->SetThreadPoolWorker(Work);
		return ZipOperation;
	}

	//Background Thread convenience functions
	UZipOperation* UnzipOnBGThreadWithFormat(const FString& ArchivePath, const FString& DestinationDirectory, const UObject* ProgressDelegate, ZipUtilityCompressionFormat Format)
	{
		UZipOperation* ZipOperation = NewObject<UZipOperation>();

		IQueuedWork* Work = RunLambdaOnThreadPool([ProgressDelegate, ArchivePath, DestinationDirectory, Format, ZipOperation] 
		{
			SevenZipCallbackHandler PrivateCallback;
			PrivateCallback.ProgressDelegate = (UObject*)ProgressDelegate;
			ZipOperation->SetCallbackHandler(&PrivateCallback);

			//UE_LOG(LogClass, Log, TEXT("path is: %s"), *path);
			SevenZipExtractor Extractor(SZLib, *ArchivePath);

			if (Format == COMPRESSION_FORMAT_UNKNOWN)
			{
				if (!Extractor.DetectCompressionFormat())
				{
					UE_LOG(LogTemp, Log, TEXT("auto-compression detection did not succeed, passing in unknown format to 7zip library."));
				}
			}
			else
			{
				Extractor.SetCompressionFormat(libZipFormatFromUEFormat(Format));
			}

			Extractor.ExtractArchive(*DestinationDirectory, &PrivateCallback);

			// Null out the callback handler now that we're exiting
			ZipOperation->SetCallbackHandler(nullptr);
		});

		ZipOperation->SetThreadPoolWorker(Work);
		return ZipOperation;
	}

	void ListOnBGThread(const FString& Path, const FString& Directory, const UObject* ListDelegate, ZipUtilityCompressionFormat Format)
	{
		//RunLongLambdaOnAnyThread - this shouldn't take long, but if it lags, swap the lambda methods
		RunLambdaOnAnyThread([ListDelegate, Path, Format, Directory] {
			SevenZipCallbackHandler PrivateCallback;
			PrivateCallback.ProgressDelegate = (UObject*)ListDelegate;
			SevenZipLister Lister(SZLib, *Path);

			if (Format == COMPRESSION_FORMAT_UNKNOWN) 
			{
				if (!Lister.DetectCompressionFormat())
				{
					UE_LOG(LogTemp, Log, TEXT("auto-compression detection did not succeed, passing in unknown format to 7zip library."));
				}
			}
			else
			{
				Lister.SetCompressionFormat(libZipFormatFromUEFormat(Format));
			}

			if (!Lister.ListArchive(&PrivateCallback))
			{
				// If ListArchive returned false, it was most likely because the compression format was unsupported
				// Call OnDone with a failure message, make sure to call this on the game thread.
				if (IZipUtilityInterface* ZipInterface = Cast<IZipUtilityInterface>((UObject*)ListDelegate))
				{
					UE_LOG(LogClass, Warning, TEXT("ZipUtility: Unknown failure for list operation on %s"), *Path);
					UZipFileFunctionLibrary::RunLambdaOnGameThread([ZipInterface, ListDelegate, Path] 
					{
						ZipInterface->Execute_OnDone((UObject*)ListDelegate, *Path, EZipUtilityCompletionState::FAILURE_UNKNOWN);
					});
				}
			}
		});
	}

	UZipOperation* ZipOnBGThread(const FString& Path, const FString& FileName, const FString& Directory, const UObject* ProgressDelegate, ZipUtilityCompressionFormat UeCompressionformat, ZipUtilityCompressionLevel UeCompressionlevel)
	{
		UZipOperation* ZipOperation = NewObject<UZipOperation>();

		IQueuedWork* Work = RunLambdaOnThreadPool([ProgressDelegate, FileName, Path, UeCompressionformat, UeCompressionlevel, Directory, ZipOperation] 
		{
			SevenZipCallbackHandler PrivateCallback;
			PrivateCallback.ProgressDelegate = (UObject*)ProgressDelegate;
			ZipOperation->SetCallbackHandler(&PrivateCallback);

			//Set the zip format
			ZipUtilityCompressionFormat UeFormat = UeCompressionformat;

			if (UeFormat == COMPRESSION_FORMAT_UNKNOWN) 
			{
				UeFormat = COMPRESSION_FORMAT_ZIP;
			}
			//Disallow creating .rar archives as per unrar restriction, this won't work anyway so redirect to 7z
			else if (UeFormat == COMPRESSION_FORMAT_RAR) 
			{
				UE_LOG(LogClass, Warning, TEXT("ZipUtility: Rar compression not supported for creating archives, re-targeting as 7z."));
				UeFormat = COMPRESSION_FORMAT_SEVEN_ZIP;
			}
			
			//concatenate the output filename
			FString OutputFileName = FString::Printf(TEXT("%s/%s%s"), *Directory, *FileName, *defaultExtensionFromUEFormat(UeFormat));
			//UE_LOG(LogClass, Log, TEXT("\noutputfile is: <%s>\n path is: <%s>"), *outputFileName, *path);
			
			SevenZipCompressor compressor(SZLib, *ReversePathSlashes(OutputFileName));
			compressor.SetCompressionFormat(libZipFormatFromUEFormat(UeFormat));
			compressor.SetCompressionLevel(libZipLevelFromUELevel(UeCompressionlevel));

			if (PathIsDirectory(*Path))
			{
				//UE_LOG(LogClass, Log, TEXT("Compressing Folder"));
				compressor.CompressDirectory(*ReversePathSlashes(Path), &PrivateCallback);
			}
			else
			{
				//UE_LOG(LogClass, Log, TEXT("Compressing File"));
				compressor.CompressFile(*ReversePathSlashes(Path), &PrivateCallback);
			}

			// Null out the callback handler
			ZipOperation->SetCallbackHandler(nullptr);
			//Todo: expand to support zipping up contents of current folder
			//compressor.CompressFiles(*ReversePathSlashes(path), TEXT("*"),  &PrivateCallback);
		});
		ZipOperation->SetThreadPoolWorker(Work);
		return ZipOperation;
	}

}//End private namespace

UZipFileFunctionLibrary::UZipFileFunctionLibrary(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	UE_LOG(LogTemp, Log, TEXT("DLLPath is: %s"), *DLLPath());
	SZLib.Load(*DLLPath());
}

UZipFileFunctionLibrary::~UZipFileFunctionLibrary()
{
	SZLib.Free();
}

bool UZipFileFunctionLibrary::UnzipFileNamed(const FString& archivePath, const FString& Name, UObject* ZipUtilityInterfaceDelegate, ZipUtilityCompressionFormat format /*= COMPRESSION_FORMAT_UNKNOWN*/)
{	
	UZipFileFunctionInternalCallback* InternalCallback = NewObject<UZipFileFunctionInternalCallback>();
	InternalCallback->SetFlags(RF_MarkAsRootSet);
	InternalCallback->SetCallback(Name, ZipUtilityInterfaceDelegate, format);

	ListFilesInArchive(archivePath, InternalCallback, format);

	return true;
}

bool UZipFileFunctionLibrary::UnzipFileNamedTo(const FString& archivePath, const FString& Name, const FString& destinationPath, UObject* ZipUtilityInterfaceDelegate, ZipUtilityCompressionFormat format /*= COMPRESSION_FORMAT_UNKNOWN*/)
{
	UZipFileFunctionInternalCallback* InternalCallback = NewObject<UZipFileFunctionInternalCallback>();
	InternalCallback->SetFlags(RF_MarkAsRootSet);
	InternalCallback->SetCallback(Name, destinationPath, ZipUtilityInterfaceDelegate, format);

	ListFilesInArchive(archivePath, InternalCallback, format);

	return true;
}

UZipOperation* UZipFileFunctionLibrary::UnzipFilesTo(const TArray<int32> fileIndices, const FString & archivePath, const FString & destinationPath, UObject * ZipUtilityInterfaceDelegate, ZipUtilityCompressionFormat format)
{
	return UnzipFilesOnBGThreadWithFormat(fileIndices, archivePath, destinationPath, ZipUtilityInterfaceDelegate, format);
}

UZipOperation* UZipFileFunctionLibrary::UnzipFiles(const TArray<int32> fileIndices, const FString & ArchivePath, UObject * ZipUtilityInterfaceDelegate, ZipUtilityCompressionFormat format)
{
	FString Directory;
	FString FileName;

	//Check Directory validity
	if (!IsValidDirectory(Directory, FileName, ArchivePath))
	{
		return nullptr;
	}
		

	if (fileIndices.Num() == 0)
	{
		return nullptr;
	}

	return UnzipFilesTo(fileIndices, ArchivePath, Directory, ZipUtilityInterfaceDelegate, format);
}

UZipOperation* UZipFileFunctionLibrary::Unzip(const FString& ArchivePath, UObject* ZipUtilityInterfaceDelegate, ZipUtilityCompressionFormat Format /*= COMPRESSION_FORMAT_UNKNOWN*/)
{
	FString Directory;
	FString FileName;

	//Check Directory validity
	if (!IsValidDirectory(Directory, FileName, ArchivePath) || !UWindowsFileUtilityFunctionLibrary::DoesFileExist(ArchivePath))
	{
		((IZipUtilityInterface*)ZipUtilityInterfaceDelegate)->Execute_OnDone((UObject*)ZipUtilityInterfaceDelegate, ArchivePath, EZipUtilityCompletionState::FAILURE_NOT_FOUND);
		return nullptr;
	}

	return UnzipTo(ArchivePath, Directory, ZipUtilityInterfaceDelegate, Format);
}

UZipOperation* UZipFileFunctionLibrary::UnzipWithLambda(const FString& ArchivePath, TFunction<void()> OnDoneCallback, TFunction<void(float)> OnProgressCallback, ZipUtilityCompressionFormat Format)
{
	UZULambdaDelegate* LambdaDelegate = NewObject<UZULambdaDelegate>();
	LambdaDelegate->AddToRoot();
	LambdaDelegate->SetOnDoneCallback([LambdaDelegate, OnDoneCallback]() 
	{
		OnDoneCallback();
		LambdaDelegate->RemoveFromRoot();
	});
	LambdaDelegate->SetOnProgessCallback(OnProgressCallback);

	return Unzip(ArchivePath, LambdaDelegate, Format);
}


UZipOperation* UZipFileFunctionLibrary::UnzipTo(const FString& ArchivePath, const FString& DestinationPath, UObject* ZipUtilityInterfaceDelegate, ZipUtilityCompressionFormat Format)
{
	return UnzipOnBGThreadWithFormat(ArchivePath, DestinationPath, ZipUtilityInterfaceDelegate, Format);
}

UZipOperation* UZipFileFunctionLibrary::Zip(const FString& ArchivePath, UObject* ZipUtilityInterfaceDelegate, ZipUtilityCompressionFormat Format, TEnumAsByte<ZipUtilityCompressionLevel> Level)
{
	FString Directory;
	FString FileName;

	//Check Directory and File validity
	if (!IsValidDirectory(Directory, FileName, ArchivePath) || !UWindowsFileUtilityFunctionLibrary::DoesFileExist(ArchivePath))
	{
		((IZipUtilityInterface*)ZipUtilityInterfaceDelegate)->Execute_OnDone((UObject*)ZipUtilityInterfaceDelegate, ArchivePath, EZipUtilityCompletionState::FAILURE_NOT_FOUND);
		return nullptr;
	}

	return ZipOnBGThread(ArchivePath, FileName, Directory, ZipUtilityInterfaceDelegate, Format, Level);
}

UZipOperation* UZipFileFunctionLibrary::ZipWithLambda(const FString& ArchivePath, TFunction<void()> OnDoneCallback, TFunction<void(float)> OnProgressCallback /*= nullptr*/, ZipUtilityCompressionFormat Format /*= COMPRESSION_FORMAT_UNKNOWN*/, TEnumAsByte<ZipUtilityCompressionLevel> Level /*=COMPRESSION_LEVEL_NORMAL*/)
{
	UZULambdaDelegate* LambdaDelegate = NewObject<UZULambdaDelegate>();
	LambdaDelegate->AddToRoot();
	LambdaDelegate->SetOnDoneCallback([OnDoneCallback, LambdaDelegate]() 
	{
		OnDoneCallback();
		LambdaDelegate->RemoveFromRoot();
	});
	LambdaDelegate->SetOnProgessCallback(OnProgressCallback);

	return Zip(ArchivePath, LambdaDelegate, Format);
}

bool UZipFileFunctionLibrary::ListFilesInArchive(const FString& path, UObject* ListDelegate, ZipUtilityCompressionFormat format)
{
	FString Directory;
	FString FileName;

	//Check Directory validity
	if (!IsValidDirectory(Directory, FileName, path))
	{
		return false;
	}

	ListOnBGThread(path, Directory, ListDelegate, format);
	return true;
}

FGraphEventRef UZipFileFunctionLibrary::RunLambdaOnGameThread(TFunction< void()> InFunction)
{
	return FFunctionGraphTask::CreateAndDispatchWhenReady(InFunction, TStatId(), nullptr, ENamedThreads::GameThread);
}
