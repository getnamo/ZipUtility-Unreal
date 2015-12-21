#pragma once

#include "ZipUtilityInterface.h"
#include "ZipFileFunctionLibrary.generated.h"

UENUM(BlueprintType)
enum ZipUtilityCompressionFormat
{
	COMPRESSION_FORMAT_UNKNOWN,
	COMPRESSION_FORMAT_SEVEN_ZIP,
	COMPRESSION_FORMAT_ZIP,
	COMPRESSION_FORMAT_GZIP,
	COMPRESSION_FORMAT_BZIP2,
	COMPRESSION_FORMAT_RAR,
	COMPRESSION_FORMAT_TAR,
	COMPRESSION_FORMAT_ISO,
	COMPRESSION_FORMAT_CAB,
	COMPRESSION_FORMAT_LZMA,
	COMPRESSION_FORMAT_LZMA86
};

class SevenZipCallbackHandler;

UCLASS(ClassGroup = ZipUtility, Blueprintable)
class UZipFileFunctionLibrary : public UBlueprintFunctionLibrary
{

public:
	GENERATED_UCLASS_BODY()
	~UZipFileFunctionLibrary();

	/* Unzips archive at path, automatically determines compression. Calls ZipUtilityInterface progress events. */
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static bool Unzip(const FString& path, UObject* ZipUtilityInterfaceDelegate); //TScriptInterface<ISevenZipInterface>, has issues with conversion to UObject*

	/* Unzips archive at path with the specified compression format. Calls ZipUtilityInterface progress events.*/
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static bool UnzipWithFormat(const FString& path, UObject* ZipUtilityInterfaceDelegate, TEnumAsByte<ZipUtilityCompressionFormat> format = COMPRESSION_FORMAT_UNKNOWN);

	/* Compresses the file or folder given at path and places the file in the same root folder. Calls ZipUtilityInterface progress events. Not all formats are supported for compression.*/
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static bool Zip(const FString& path, UObject* ZipUtilityInterfaceDelegate, TEnumAsByte<ZipUtilityCompressionFormat> format = COMPRESSION_FORMAT_SEVEN_ZIP);

	/*Queries Archive content list, calls ZipUtilityInterface list events (OnFileFound)*/
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static bool ListFilesInArchive(const FString& path, UObject* ZipUtilityInterfaceDelegate);

	/*Expects full path including name. you can use this function to rename files.*/
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static bool MoveFileTo(const FString& from, const FString& to);

	/*Expects full path including folder name.*/
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static bool CreateDirectoryAt(const FString& fullPath);

private:
};