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
class ZIPUTILITY_API UZipFileFunctionLibrary : public UBlueprintFunctionLibrary
{

public:
	GENERATED_UCLASS_BODY()
	~UZipFileFunctionLibrary();

	/* Unzips archive at current path. Automatically determines compression if unknown. Calls ZipUtilityInterface progress events. */
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static bool Unzip(const FString& ArchivePath, UObject* ZipUtilityInterfaceDelegate, TEnumAsByte<ZipUtilityCompressionFormat> format = COMPRESSION_FORMAT_UNKNOWN);

	/* Unzips archive at destination path. Automatically determines compression if unknown. Calls ZipUtilityInterface progress events. */
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static bool UnzipTo(const FString& ArchivePath, const FString& DestinationPath, UObject* ZipUtilityInterfaceDelegate, TEnumAsByte<ZipUtilityCompressionFormat> format = COMPRESSION_FORMAT_UNKNOWN);

	/* Compresses the file or folder given at path and places the file in the same root folder. Calls ZipUtilityInterface progress events. Not all formats are supported for compression.*/
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static bool Zip(const FString& FileOrFolderPath, UObject* ZipUtilityInterfaceDelegate, TEnumAsByte<ZipUtilityCompressionFormat> format = COMPRESSION_FORMAT_SEVEN_ZIP);

	/*Queries Archive content list, calls ZipUtilityInterface list events (OnFileFound)*/
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static bool ListFilesInArchive(const FString& ArchivePath, UObject* ZipUtilityInterfaceDelegate);

	/*Expects full path including name. you can use this function to rename files.*/
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static bool MoveFileTo(const FString& From, const FString& To);

	/*Expects full path including folder name.*/
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static bool CreateDirectoryAt(const FString& FullPath);

	/*Deletes file (not directory). Expects full path.*/
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static bool DeleteFileAt(const FString& FullPath);

	/*Deletes empty folders only. Expects full path.*/
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static bool DeleteEmptyFolder(const FString& FullPath);

	/*Dangerous function, not exposed to blueprint. */
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static bool DeleteFolderRecursively(const FString& FullPath);
};