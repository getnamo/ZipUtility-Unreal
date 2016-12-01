#pragma once

#include "LambdaRunnable.h"
#include "WindowsFileUtilityFunctionLibrary.generated.h"

//Struct to Track which delegate is watching files
USTRUCT()
struct FWatcher
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TWeakObjectPtr<UObject> Delegate;
	
	UPROPERTY()
	FString Path;

	FLambdaRunnable* Runnable;
};

UCLASS(ClassGroup = WindowsFileUtility, Blueprintable)
class WINDOWSFILEUTILITY_API UWindowsFileUtilityFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	/*Expects full path including name. you can use this function to rename files.*/
	UFUNCTION(BlueprintCallable, Category = WindowsFileUtility)
	static bool MoveFileTo(const FString& From, const FString& To);

	/*Expects full path including folder name.*/
	UFUNCTION(BlueprintCallable, Category = WindowsFileUtility)
	static bool CreateDirectoryAt(const FString& FullPath);

	/*Deletes file (not directory). Expects full path.*/
	UFUNCTION(BlueprintCallable, Category = WindowsFileUtility)
	static bool DeleteFileAt(const FString& FullPath);

	/*Deletes empty folders only. Expects full path.*/
	UFUNCTION(BlueprintCallable, Category = WindowsFileUtility)
	static bool DeleteEmptyFolder(const FString& FullPath);

	/*Dangerous function, not exposed to blueprint. */
	UFUNCTION(BlueprintCallable, Category = WindowsFileUtility)
	static bool DeleteFolderRecursively(const FString& FullPath);

	UFUNCTION(BlueprintCallable, Category = WindowsFileUtility)
	static void WatchFolder(const FString& FullPath, UObject* WatcherDelegate);

	UFUNCTION(BlueprintCallable, Category = WindowsFileUtility)
	static void StopWatchingFolder(const FString& FullPath, UObject* WatcherDelegate);


private:
	static void WatchFolderOnBgThread(const FString& FullPath, UObject* WatcherDelegate);
	static TMap<FString, TArray<FWatcher>> Watchers;
};