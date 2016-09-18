#pragma once

#include "ZipUtilityInterface.h"
#include "ZipFileFunctionInternalCallback.generated.h"

UCLASS(ClassGroup = ZipUtility)
class ZIPUTILITY_API UZipFileFunctionInternalCallback : public UObject, public IZipUtilityInterface
{
	GENERATED_BODY()

private:

	/** Compression format used to unzip */
	TEnumAsByte<ZipUtilityCompressionFormat> CompressionFormat;

	/** Path of the file */
	FString File;

	FString DestinationFolder;

	/** Current File index parsed */
	int32 FileIndex = 0;

	/** Callback object */
	UPROPERTY(Transient)
	UObject* Callback;

	bool bSingleFile = false;
	bool bFileFound = false;
	bool bUnzipto = false;

public:
	UZipFileFunctionInternalCallback();

	//IZipUtilityInterface overrides
	virtual void OnProgress_Implementation(const FString& archive, float percentage, int32 bytes) override {};

	virtual void OnDone_Implementation(const FString& archive) override {};

	virtual void OnStartProcess_Implementation(const FString& archive, int32 bytes) override {};

	virtual void OnFileDone_Implementation(const FString& archive, const FString& file) override {
		UE_LOG(LogTemp, Log, TEXT("OnFileDone_Implementation")); 
	};

	virtual void OnFileFound_Implementation(const FString& archive, const FString& fileIn, int32 size) override;

	void SetCallback(const FString& FileName, UObject* CallbackIn, TEnumAsByte<ZipUtilityCompressionFormat> CompressionFormatIn = ZipUtilityCompressionFormat::COMPRESSION_FORMAT_UNKNOWN);
	void SetCallback(const FString& FileName, const FString& DestinationFolder, UObject* CallbackIn, TEnumAsByte<ZipUtilityCompressionFormat> CompressionFormatIn = ZipUtilityCompressionFormat::COMPRESSION_FORMAT_UNKNOWN);

	FORCEINLINE bool GetSingleFile() const { return bSingleFile; }
	FORCEINLINE void SetSingleFile(bool val) { bSingleFile = val; }
};