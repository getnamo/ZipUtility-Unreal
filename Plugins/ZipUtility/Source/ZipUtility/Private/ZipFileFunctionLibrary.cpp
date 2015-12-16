#include "ZipUtilityPrivatePCH.h"
#include "ZipFileFunctionLibrary.h"

#include "ListCallback.h"
#include "ProgressCallback.h"
#include "7zpp.h"

using namespace SevenZip;

//Private Namespace
namespace{

	class SevenZipCallbackHandler : public ListCallback, public ProgressCallback
	{
	public:
		virtual void OnProgress(float progress) override
		{

		}
		virtual void OnDone() override
		{

		}
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

		FString dllString = FString("7z.dll");	//Using 7z.dll: GNU LGPL + unRAR restriction
												//FString dllString = FString("7za.dll");	//Using 7za.dll: GNU LGPL license, crucially doesn't support .zip

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

	ZipUtilityCompressionFormat formatForFile(const FString &fileName)
	{
		//Continue here, handle looking up compression for file ending
		return COMPRESSION_FORMAT_ZIP;	//for now pretend its a zip, always

										//fileName.Split(TEXT("."), &);
										//return CompressionFormat::Unknown;
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

bool UZipFileFunctionLibrary::Unzip(const FString& path, TScriptInterface<ISevenZipInterface> progressDelegate)
{
	return UnzipWithFormat(path, progressDelegate, COMPRESSION_FORMAT_UNKNOWN);
}

bool UZipFileFunctionLibrary::UnzipWithFormat(const FString& path, TScriptInterface<ISevenZipInterface> progressDelegate, TEnumAsByte<ZipUtilityCompressionFormat> format)
{
	//temp for hello world like testing
	FString directory;
	FString fileName;

	//Check directory validity
	if (!isValidDirectory(directory, fileName, path))
		return false;

	if (format == COMPRESSION_FORMAT_UNKNOWN)
		format = formatForFile(fileName);

	SevenZipExtractor extractor(SZLib, *path);
	extractor.SetCompressionFormat(libZipFormatFromUEFormat(format));

	//CONTINUE HERE, test extraction, forward progress to some delegate event or interface
	return extractor.ExtractArchive(*directory, &PrivateCallback);
}

bool UZipFileFunctionLibrary::Zip(const FString& path, TScriptInterface<ISevenZipInterface> progressDelegate, TEnumAsByte<ZipUtilityCompressionFormat> format)
{
	//CONTINUE here add basic zip functionality
	/*
	Also see if we can't get this to become event driven so we know when it has finished and/or progress/etc
	*/
	return false;
}

bool UZipFileFunctionLibrary::GetFileList(const FString& path, TScriptInterface<ISevenZipInterface> listDelegate)
{
	//temp for hello world like testing
	FString directory;
	FString fileName;

	//Check directory validity
	if (!isValidDirectory(directory, fileName, path))
		return false;

	ZipUtilityCompressionFormat format = formatForFile(fileName);

	TArray<FString> list;
	/*
	TODO: this needs to function, once we've update the library to function expand this
	*/
	return false;
}

ZipUtilityCompressionFormat UZipFileFunctionLibrary::formatForFileName(const FString& fileName)
{
	//TODO: get the ending, switch to the correct default formats.
	//fileName.Split(TEXT("."), &);

	return COMPRESSION_FORMAT_UNKNOWN;
}

