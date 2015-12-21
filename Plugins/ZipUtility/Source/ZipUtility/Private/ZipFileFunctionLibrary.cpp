#include "ZipUtilityPrivatePCH.h"
#include "ZipFileFunctionLibrary.h"

#include "ListCallback.h"
#include "ProgressCallback.h"
#include "7zpp.h"
#include "LambdaRunnable.h"

#include "AllowWindowsPlatformTypes.h" 
#include "Windows.h"
#include "HideWindowsPlatformTypes.h"

using namespace SevenZip;

//Private Namespace
namespace{

	//Threaded Lambda convenience wrappers - Task graph is only suitable for short duration lambdas, but doesn't incur thread overhead
	FGraphEventRef RunLambdaOnGameThread(TFunction< void()> InFunction)
	{
		return FFunctionGraphTask::CreateAndDispatchWhenReady(InFunction, TStatId(), nullptr, ENamedThreads::GameThread);
	}

	FGraphEventRef RunLambdaOnAnyThread(TFunction< void()> InFunction)
	{
		return FFunctionGraphTask::CreateAndDispatchWhenReady(InFunction, TStatId(), nullptr, ENamedThreads::AnyThread);
	}

	//Uses proper threading, for any task that may run longer than about 2 seconds.
	void RunLongLambdaOnAnyThread(TFunction< void()> InFunction)
	{
		FLambdaRunnable::RunLambdaOnBackGroundThread(InFunction);
	}

	class SevenZipCallbackHandler : public ListCallback, public ProgressCallback
	{
	public:
		//Event Callbacks from the 7zpp library - Forward them to our UE listener
		
		//For now disabled, we use on file done as a progress indicator, this is good enough for generic progress.
		/*virtual void OnProgress(uint64 progress) override
		{
			const UObject* interfaceDelegate = progressDelegate;
			const uint64 bytesConst = progress;

			RunLambdaOnGameThread([interfaceDelegate, bytesConst] {
				//UE_LOG(LogClass, Log, TEXT("Progress: %d bytes"), progress);
				((ISevenZipInterface*)interfaceDelegate)->Execute_OnProgress((UObject*)interfaceDelegate, bytesConst);
			});
		}*/

		virtual void OnDone() override
		{
			const UObject* interfaceDelegate = progressDelegate;

			RunLambdaOnGameThread([interfaceDelegate] {
				//UE_LOG(LogClass, Log, TEXT("All Done!"));
				((IZipUtilityInterface*)interfaceDelegate)->Execute_OnDone((UObject*)interfaceDelegate);
			});
		}

		virtual void OnFileDone(TString filePath, uint64 bytes) override
		{
			const UObject* interfaceDelegate = progressDelegate;
			const TString filePathConst = filePath;
			const uint64 bytesConst = bytes;

			RunLambdaOnGameThread([interfaceDelegate, filePathConst, bytesConst] {
				//UE_LOG(LogClass, Log, TEXT("File Done: %s, %d bytes"), filePathConst.c_str(), bytesConst);
				((IZipUtilityInterface*)interfaceDelegate)->Execute_OnFileDone((UObject*)interfaceDelegate, filePathConst.c_str());
			});

			//Handle byte decrementing
			if (bytes > 0) {
				BytesLeft -= bytes;
				const float ProgressPercentage = ((double)(TotalBytes-BytesLeft) / (double)TotalBytes) * 100;

				RunLambdaOnGameThread([interfaceDelegate, ProgressPercentage] {
					//UE_LOG(LogClass, Log, TEXT("Progress: %d bytes"), progress);
					((IZipUtilityInterface*)interfaceDelegate)->Execute_OnProgress((UObject*)interfaceDelegate, ProgressPercentage);
				});
			}
		}

		virtual void OnStartWithTotal(unsigned __int64 totalBytes) 
		{
			TotalBytes = totalBytes;
			BytesLeft = TotalBytes;

			const UObject* interfaceDelegate = progressDelegate;
			const uint64 bytesConst = TotalBytes;

			RunLambdaOnGameThread([interfaceDelegate, bytesConst] {
				//UE_LOG(LogClass, Log, TEXT("Starting with %d bytes"), bytesConst);
				((IZipUtilityInterface*)interfaceDelegate)->Execute_OnStartProcess((UObject*)interfaceDelegate,bytesConst);
			});
		}

		virtual void OnFileFound(WCHAR* path, int size)
		{
			const UObject* interfaceDelegate = progressDelegate;
			const uint64 bytesConst = TotalBytes;
			const FString pathString = FString(path);

			RunLambdaOnGameThread([interfaceDelegate, pathString, bytesConst] {
				//UE_LOG(LogClass, Log, TEXT("File Found: %s"), path);
				((IZipUtilityInterface*)interfaceDelegate)->Execute_OnFileFound((UObject*)interfaceDelegate, pathString, bytesConst);
			});

			
		}

		uint64 BytesLeft = 0;
		uint64 TotalBytes = 0;
		UObject* progressDelegate;
	};

	//Private static vars
	SevenZipCallbackHandler PrivateCallback;
	SevenZipLibrary SZLib;

	//Utility functions
	FString UtilityGameFolder()
	{
		return FPaths::ConvertRelativePathToFull(FPaths::GameDir());
	}

	FString DLLPath()
	{
#if _WIN64

		FString PlatformString = FString(TEXT("Win64"));
#else
		FString PlatformString = FString(TEXT("Win32"));
#endif
		//Swap these to change which license you wish to fall under for zip-utility

		FString dllString = FString("7z.dll");		//Using 7z.dll: GNU LGPL + unRAR restriction
		//FString dllString = FString("7za.dll");	//Using 7za.dll: GNU LGPL license, crucially doesn't support .zip out of the box

		return FPaths::Combine(*UtilityGameFolder(), TEXT("Plugins/ZipUtility/ThirdParty/7zpp/dll"), *PlatformString, *dllString);
	}

	FString ReversePathSlashes(FString forwardPath)
	{
		return forwardPath.Replace(TEXT("/"), TEXT("\\"));
	}

	bool isValidDirectory(FString& directory, FString& fileName, const FString& path)
	{
		bool found = path.Split(TEXT("/"), &directory, &fileName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		//try a back split
		if (!found)
		{
			found = path.Split(TEXT("\\"), &directory, &fileName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		}

		//No valid directory found
		if (!found)
			return false;
		else
			return true;
	}

	SevenZip::CompressionFormatEnum libZipFormatFromUEFormat(ZipUtilityCompressionFormat ueFormat) {
		switch (ueFormat)
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
	void UnzipOnBGThreadWithFormat(const FString& path, const FString& directory, const UObject* progressDelegate, ZipUtilityCompressionFormat format)
	{
		PrivateCallback.progressDelegate = (UObject*)progressDelegate;

		RunLongLambdaOnAnyThread([progressDelegate, path, format, directory] {

			//UE_LOG(LogClass, Log, TEXT("path is: %s"), *path);
			SevenZipExtractor extractor(SZLib, *path);

			if (format == COMPRESSION_FORMAT_UNKNOWN) {
				if (!extractor.DetectCompressionFormat())
				{
					extractor.SetCompressionFormat(SevenZip::CompressionFormat::Zip);
				}
			}
			else
			{
				extractor.SetCompressionFormat(libZipFormatFromUEFormat(format));
			}

			extractor.ExtractArchive(*directory, &PrivateCallback);
				
		});
	}

	void ListOnBGThread(const FString& path, const FString& directory, const UObject* progressDelegate, ZipUtilityCompressionFormat format)
	{
		PrivateCallback.progressDelegate = (UObject*)progressDelegate;

		//RunLongLambdaOnAnyThread - this shouldn't take long, but if it lags, swap the lambda methods
		RunLambdaOnAnyThread([progressDelegate, path, format, directory] {

			SevenZipLister lister(SZLib, *path);

			if (format == COMPRESSION_FORMAT_UNKNOWN) {
				if (!lister.DetectCompressionFormat())
				{
					lister.SetCompressionFormat(SevenZip::CompressionFormat::Zip);
				}
			}
			else
			{
				lister.SetCompressionFormat(libZipFormatFromUEFormat(format));
			}

			lister.ListArchive(&PrivateCallback);

		});
	}

	void ZipOnBGThread(const FString& path, const FString& fileName, const FString& directory, const UObject* progressDelegate, ZipUtilityCompressionFormat format)
	{
		PrivateCallback.progressDelegate = (UObject*)progressDelegate;

		RunLongLambdaOnAnyThread([progressDelegate, fileName, path, format, directory] {
			//Set the zip format
			ZipUtilityCompressionFormat ueFormat = format;

			if (ueFormat == COMPRESSION_FORMAT_UNKNOWN) {
				ueFormat = COMPRESSION_FORMAT_ZIP;
			}
			//Disallow creating .rar archives as per unrar restriction, this won't work anyway so redirect to 7z
			else if (ueFormat == COMPRESSION_FORMAT_RAR) {
				UE_LOG(LogClass, Warning, TEXT("ZipUtility: Rar compression not supported for creating archives, re-targeting as 7z."));
				ueFormat = COMPRESSION_FORMAT_SEVEN_ZIP;
			}
			
			//concatenate the output filename
			FString outputFileName = FString::Printf(TEXT("%s/%s%s"), *directory, *fileName, *defaultExtensionFromUEFormat(ueFormat));
			//UE_LOG(LogClass, Log, TEXT("\noutputfile is: <%s>\n path is: <%s>"), *outputFileName, *path);
			
			SevenZipCompressor compressor(SZLib, *outputFileName);
			compressor.SetCompressionFormat(libZipFormatFromUEFormat(ueFormat));

			if (PathIsDirectory(*path))
			{
				//UE_LOG(LogClass, Log, TEXT("Compressing Folder"));
				compressor.CompressDirectory(*ReversePathSlashes(path), &PrivateCallback);
			}
			else
			{
				//UE_LOG(LogClass, Log, TEXT("Compressing File"));
				compressor.CompressFile(*ReversePathSlashes(path), &PrivateCallback);
			}

			//Todo: expand to support zipping up contents of folder
			//compressor.CompressFiles(*ReversePathSlashes(path), TEXT("*"),  &PrivateCallback);

		});
	}

}//End private namespace

UZipFileFunctionLibrary::UZipFileFunctionLibrary(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	SZLib.Load(*DLLPath());
}

UZipFileFunctionLibrary::~UZipFileFunctionLibrary()
{
	SZLib.Free();
}

bool UZipFileFunctionLibrary::Unzip(const FString& path, UObject* progressDelegate)
{
	return UnzipWithFormat(path, progressDelegate, COMPRESSION_FORMAT_UNKNOWN);
}

bool UZipFileFunctionLibrary::UnzipWithFormat(const FString& path, UObject* progressDelegate, TEnumAsByte<ZipUtilityCompressionFormat> format)
{
	FString directory;
	FString fileName;

	//Check directory validity
	if (!isValidDirectory(directory, fileName, path))
		return false;

	UnzipOnBGThreadWithFormat(path, directory, progressDelegate, format);

	return true;
}

bool UZipFileFunctionLibrary::Zip(const FString& path, UObject* progressDelegate, TEnumAsByte<ZipUtilityCompressionFormat> format)
{
	FString directory;
	FString fileName;

	//Check directory validity
	if (!isValidDirectory(directory, fileName, path))
		return false;

	ZipOnBGThread(path, fileName, directory, progressDelegate, format);
	return true;
}

bool UZipFileFunctionLibrary::ListFilesInArchive(const FString& path, UObject* listDelegate)
{
	FString directory;
	FString fileName;

	//Check directory validity
	if (!isValidDirectory(directory, fileName, path))
		return false;

	ListOnBGThread(path, directory, listDelegate, COMPRESSION_FORMAT_UNKNOWN);
	return true;
}

bool UZipFileFunctionLibrary::MoveFileTo(const FString& from, const FString& to)
{
	//Using windows api
	return 0 != MoveFileW(*from, *to);
}


bool UZipFileFunctionLibrary::CreateDirectoryAt(const FString& fullPath)
{
	//Using windows api
	return 0 != CreateDirectoryW(*fullPath, NULL);
}

