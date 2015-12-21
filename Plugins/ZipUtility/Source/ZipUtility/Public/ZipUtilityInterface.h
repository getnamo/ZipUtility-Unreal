// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ZipUtilityInterface.generated.h"

UINTERFACE(MinimalAPI)
class UZipUtilityInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class IZipUtilityInterface
{
	GENERATED_IINTERFACE_BODY()

public:

	/**
	* Called during process as it completes. Currently updates on per file progress.
	* @param percentage - percentage done
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = ZipUtilityProgressEvents)
		void OnProgress(float percentage);

	/**
	* Called when whole process is complete (e.g. unzipping completed on archive)
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = ZipUtilityProgressEvents)
		void OnDone();

	/**
	* Called at beginning of process (NB this only supports providing size information for up to 2gb) TODO: fix 32bit BP size issue
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = ZipUtilityProgressEvents)
		void OnStartProcess(int32 bytes);

	/**
	* Called when file process is complete
	* @param path - path of the file that finished
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = ZipUtilityProgressEvents)
		void OnFileDone(const FString& path);


	/**
	* Called when a file is found in the archive (e.g. listing the entries in the archive)
	* @param path - path of file
	* @param size - compressed size
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = ZipUtilityListEvents)
		void OnFileFound(const FString& path, int32 size);
};