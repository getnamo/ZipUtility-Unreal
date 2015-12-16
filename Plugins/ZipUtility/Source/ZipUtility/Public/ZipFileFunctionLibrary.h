#pragma once

#include "SevenZipInterface.h"
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

	/*UFUNCTION(BlueprintPure, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", DisplayName = "Create BluJSON Obj", CompactNodeTitle = "JSON", Keywords = "new create blu eye blui json"), Category = Blu)
		static UBluJsonObj* NewBluJSONObj(UObject* WorldContextObject);*/

	UFUNCTION(BlueprintCallable, Category = Zipper)
	static bool Unzip(const FString& path, TScriptInterface<ISevenZipInterface> progressDelegate);

	UFUNCTION(BlueprintCallable, Category = Zipper)
	static bool UnzipWithFormat(const FString& path, TScriptInterface<ISevenZipInterface> progressDelegate, TEnumAsByte<ZipUtilityCompressionFormat> format = COMPRESSION_FORMAT_UNKNOWN);

	UFUNCTION(BlueprintCallable, Category = Zipper)
	static bool Zip(const FString& path, TScriptInterface<ISevenZipInterface> progressDelegate, TEnumAsByte<ZipUtilityCompressionFormat> format = COMPRESSION_FORMAT_SEVEN_ZIP);

	UFUNCTION(BlueprintCallable, Category = Zipper)
	static bool GetFileList(const FString& path, TScriptInterface<ISevenZipInterface> listDelegate);

private:
	static ZipUtilityCompressionFormat formatForFileName(const FString& fileName);
	
};