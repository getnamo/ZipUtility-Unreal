#pragma once

#include "ZipUtilityInterface.h"
#include "ZipOperation.h"
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


UENUM(BlueprintType)
enum ZipUtilityCompressionLevel
{
	COMPRESSION_LEVEL_NONE,
	COMPRESSION_LEVEL_FAST,
	COMPRESSION_LEVEL_NORMAL
};

class SevenZipCallbackHandler;
class UZipFileFunctionInternalCallback;

/** 
 A blueprint function library encapsulating all zip operations for both C++ and blueprint use. 
 For some operations a UZipOperation object may be returned, if you're interested in it, ensure
 you guard it from garbage collection by e.g. storing it as a UProperty, otherwise you may safely
 ignore it.
*/

UCLASS(ClassGroup = ZipUtility, Blueprintable)
class ZIPUTILITY_API UZipFileFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	~UZipFileFunctionLibrary();
	
	/* Unzips file in archive containing Name via ListFilesInArchive/UnzipFiles. Automatically determines compression if unknown. Calls ZipUtilityInterface progress events. */
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static bool UnzipFileNamed(const FString& archivePath, const FString& Name, UObject* ZipUtilityInterfaceDelegate, ZipUtilityCompressionFormat format = COMPRESSION_FORMAT_UNKNOWN);
	
	/* Unzips file in archive containing Name at destination path via ListFilesInArchive/UnzipFilesTo. Automatically determines compression if unknown. Calls ZipUtilityInterface progress events. */
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static bool UnzipFileNamedTo(const FString& archivePath, const FString& Name, const FString& destinationPath, UObject* ZipUtilityInterfaceDelegate, ZipUtilityCompressionFormat format = COMPRESSION_FORMAT_UNKNOWN);

	/* Unzips the given file indexes in archive at destination path. Automatically determines compression if unknown. Calls ZipUtilityInterface progress events. */
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static UZipOperation* UnzipFilesTo(const TArray<int32> fileIndices, const FString& archivePath, const FString& destinationPath, UObject* ZipUtilityInterfaceDelegate, ZipUtilityCompressionFormat format = COMPRESSION_FORMAT_UNKNOWN);

	/* Unzips the given file indexes in archive at current path. Automatically determines compression if unknown. Calls ZipUtilityInterface progress events. */
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static UZipOperation* UnzipFiles(const TArray<int32> fileIndices, const FString& ArchivePath, UObject* ZipUtilityInterfaceDelegate, ZipUtilityCompressionFormat format = COMPRESSION_FORMAT_UNKNOWN);

	/* Unzips archive at current path. Automatically determines compression if unknown. Calls ZipUtilityInterface progress events. */
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static UZipOperation* Unzip(const FString& ArchivePath, UObject* ZipUtilityInterfaceDelegate, ZipUtilityCompressionFormat Format = COMPRESSION_FORMAT_UNKNOWN);

	/* Lambda C++ simple variant*/
	static UZipOperation* UnzipWithLambda(	const FString& ArchivePath,
									TFunction<void()> OnDoneCallback,
									TFunction<void(float)> OnProgressCallback = nullptr,
									ZipUtilityCompressionFormat format = COMPRESSION_FORMAT_UNKNOWN);

	/* Unzips archive at destination path. Automatically determines compression if unknown. Calls ZipUtilityInterface progress events. */
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static UZipOperation* UnzipTo(const FString& ArchivePath, const FString& DestinationPath, UObject* ZipUtilityInterfaceDelegate, ZipUtilityCompressionFormat format = COMPRESSION_FORMAT_UNKNOWN);

	/* Compresses the file or folder given at path and places the file in the same root folder. Calls ZipUtilityInterface progress events. Not all formats are supported for compression.*/
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static UZipOperation* Zip(	const FString& FileOrFolderPath,
						UObject* ZipUtilityInterfaceDelegate, 
						ZipUtilityCompressionFormat Format = COMPRESSION_FORMAT_SEVEN_ZIP, 
						TEnumAsByte<ZipUtilityCompressionLevel> Level = COMPRESSION_LEVEL_NORMAL);

	/* Lambda C++ simple variant*/
	static UZipOperation* ZipWithLambda(	const FString& ArchivePath,
								TFunction<void()> OnDoneCallback,
								TFunction<void(float)> OnProgressCallback = nullptr,
								ZipUtilityCompressionFormat Format = COMPRESSION_FORMAT_UNKNOWN,
								TEnumAsByte<ZipUtilityCompressionLevel> Level = COMPRESSION_LEVEL_NORMAL);


	/*Queries Archive content list, calls ZipUtilityInterface list events (OnFileFound)*/
	UFUNCTION(BlueprintCallable, Category = ZipUtility)
	static bool ListFilesInArchive(const FString& ArchivePath, UObject* ZipUtilityInterfaceDelegate, ZipUtilityCompressionFormat format = COMPRESSION_FORMAT_UNKNOWN);

	static FGraphEventRef RunLambdaOnGameThread(TFunction< void()> InFunction);
};


