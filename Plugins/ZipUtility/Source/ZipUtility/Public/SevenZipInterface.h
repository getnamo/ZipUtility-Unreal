// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SevenZipInterface.generated.h"

UINTERFACE(MinimalAPI)
class USevenZipInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class ISevenZipInterface
{
	GENERATED_IINTERFACE_BODY()

public:

	/**
	* Called when progress updates
	* @param percentage - percentage done
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = SevenZipEvents)
		void OnProgress(float percentage);

	/**
	* Called when progress is complete
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = SevenZipEvents)
		void OnDone();

	/**
	* Called at beginning of process (NB this only supports providing size information for up to 2gb) TODO: fix bp size issue
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = SevenZipEvents)
		void OnStartProcess(int32 bytes);

	/**
	* Called when file action is complete
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = SevenZipEvents)
		void OnFileDone(const FString& path);


	/**
	* Called when a file is found in the archive (e.g. listing the entries in the archive)
	* @param path - path of file
	* @param size - compress size
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = SevenZipEvents)
		void OnFileFound(const FString& path, int32 size);
};