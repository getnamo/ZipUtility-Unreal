#include "WindowsFileUtilityPrivatePCH.h"
#include "WindowsFileUtilityFunctionLibrary.h"

//static TMAP definition
TMap<FString, TArray<FWatcher>> UWindowsFileUtilityFunctionLibrary::Watchers = TMap<FString, TArray<FWatcher>>();
int TotalWatchers = 0;

UWindowsFileUtilityFunctionLibrary::UWindowsFileUtilityFunctionLibrary(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}

#if PLATFORM_WINDOWS

#include "AllowWindowsPlatformTypes.h"
#include <shellapi.h>

bool UWindowsFileUtilityFunctionLibrary::MoveFileTo(const FString& From, const FString& To)
{
	//Using windows api
	return 0 != MoveFileW(*From, *To);
}


bool UWindowsFileUtilityFunctionLibrary::CreateDirectoryAt(const FString& FullPath)
{
	//Using windows api
	return 0 != CreateDirectoryW(*FullPath, NULL);
}

bool UWindowsFileUtilityFunctionLibrary::DeleteFileAt(const FString& FullPath)
{
	//Using windows api
	return 0 != DeleteFileW(*FullPath);
}

bool UWindowsFileUtilityFunctionLibrary::DeleteEmptyFolder(const FString& FullPath)
{
	//Using windows api
	return 0 != RemoveDirectoryW(*FullPath);
}

bool IsSubPathOf(const FString& path, const FString& basePath)
{
	return path.Contains(basePath);
}

//Dangerous function not recommended to be exposed to blueprint 
bool UWindowsFileUtilityFunctionLibrary::DeleteFolderRecursively(const FString& FullPath)
{
	//Only allow user to delete folders sub-class to game folder
	if (!IsSubPathOf(FullPath, FPaths::GameDir()))
		return false;

	int len = _tcslen(*FullPath);
	TCHAR *pszFrom = new TCHAR[len + 2];
	wcscpy_s(pszFrom, len + 2, *FullPath);
	pszFrom[len] = 0;
	pszFrom[len + 1] = 0;

	SHFILEOPSTRUCT fileop;
	fileop.hwnd = NULL;    // no status display
	fileop.wFunc = FO_DELETE;  // delete operation
	fileop.pFrom = pszFrom;  // source file name as double null terminated string
	fileop.pTo = NULL;    // no destination needed
	fileop.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;  // do not prompt the user

	fileop.fAnyOperationsAborted = FALSE;
	fileop.lpszProgressTitle = NULL;
	fileop.hNameMappings = NULL;

	int ret = SHFileOperation(&fileop);
	delete[] pszFrom;
	return (ret == 0);
}

void UWindowsFileUtilityFunctionLibrary::WatchFolder(const FString& FullPath, UObject* WatcherDelegate)
{
	//Do we have an entry for this path?
	if (!Watchers.Contains(FullPath))
	{
		//Make an entry
		TArray<FWatcher> FreshList;
		Watchers.Add(FullPath, FreshList);
		Watchers[FullPath] = FreshList;
	}
	else
	{
		//if we do do we already watch from this object?
		TArray<FWatcher>& PathWatchers = Watchers[FullPath];

		for (auto Watcher : PathWatchers)
		{
			if (Watcher.Delegate == WatcherDelegate)
			{
				//Already accounted for
				UE_LOG(LogTemp, Warning, TEXT("UWindowsFileUtilityFunctionLibrary::WatchFolder Duplicate watcher ignored!"));
				return;
			}
		}
	}


	//Add to watchers
	FWatcher FreshWatcher;
	FreshWatcher.Delegate = WatcherDelegate;
	FreshWatcher.Path = FullPath;

	const FWatcher* WatcherPtr = &FreshWatcher;

	//fork this off to another process
	FLambdaRunnable* Runnable = FLambdaRunnable::RunLambdaOnBackGroundThread([FullPath, WatcherDelegate, WatcherPtr]()
	{
		 UWindowsFileUtilityFunctionLibrary::WatchFolderOnBgThread(FullPath, WatcherPtr);
	});

	FreshWatcher.Runnable = Runnable;

	TArray<FWatcher>& PathWatchers = Watchers[FullPath];
	PathWatchers.Add(FreshWatcher);
}

void UWindowsFileUtilityFunctionLibrary::StopWatchingFolder(const FString& FullPath, UObject* WatcherDelegate)
{
	//Do we have an entry?
	if (!Watchers.Contains(FullPath))
	{
		return;
	}

	//We have an entry for this path, remove our watcher
	TArray<FWatcher> PathWatchers = Watchers[FullPath];
	for (int i = 0; i < PathWatchers.Num();i++)
	{
		FWatcher& PathWatcher = PathWatchers[i];
		if (PathWatcher.Delegate == WatcherDelegate)
		{
			//Stop the runnable
			PathWatcher.ShouldRun = false;
			PathWatcher.Runnable->Stop();

			//Remove the watcher and we're done
			PathWatchers.RemoveAt(i);
			break;
		}
	}
}

void UWindowsFileUtilityFunctionLibrary::WatchFolderOnBgThread(const FString& FullPath, const FWatcher* WatcherPtr)
{
	//mostly from https://msdn.microsoft.com/en-us/library/windows/desktop/aa365261(v=vs.85).aspx
	//call the delegate when the folder changes

	//TODO: find out which file changed
	
	DWORD dwWaitStatus;
	HANDLE dwChangeHandles[2];
	TCHAR lpDrive[4];
	TCHAR lpFile[_MAX_FNAME];
	TCHAR lpExt[_MAX_EXT];

	//finding out about the notification
	FILE_NOTIFY_INFORMATION strFileNotifyInfo[1024];
	DWORD dwBytesReturned = 0;

	_tsplitpath_s(*FullPath, lpDrive, 4, NULL, 0, lpFile, _MAX_FNAME, lpExt, _MAX_EXT);
	lpDrive[2] = (TCHAR)'\\';
	lpDrive[3] = (TCHAR)'\0';

	// Watch the directory for file creation and deletion. 

	dwChangeHandles[0] = FindFirstChangeNotification(
		*FullPath,                     // directory to watch 
		TRUE,						   // watch the subtree 
		FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | 
		FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_ATTRIBUTES	| FILE_NOTIFY_CHANGE_SIZE// watch for generic file changes
		); // watch last write or file size change

	if (dwChangeHandles[0] == INVALID_HANDLE_VALUE)
	{
		UE_LOG(LogTemp, Warning, TEXT("\n ERROR: FindFirstChangeNotification function failed.\n"));
		//ExitProcess(GetLastError());
	}

	// Watch the subtree for directory creation and deletion. 

	dwChangeHandles[1] = FindFirstChangeNotification(
		lpDrive,                       // directory to watch 
		TRUE,                          // watch the subtree 
		FILE_NOTIFY_CHANGE_DIR_NAME);  // watch dir name changes 

	if (dwChangeHandles[1] == INVALID_HANDLE_VALUE)
	{
		UE_LOG(LogTemp, Warning, TEXT("\n ERROR: FindFirstChangeNotification function failed.\n"));
		//ExitProcess(GetLastError());
	}

	// Make a final validation check on our handles.
	if ((dwChangeHandles[0] == NULL) || (dwChangeHandles[1] == NULL))
	{
		UE_LOG(LogTemp, Warning, TEXT("\n ERROR: Unexpected NULL from FindFirstChangeNotification.\n"));
		//ExitProcess(GetLastError());
	}
	const FString DrivePath = FString(lpDrive);
	FString FileString;
	FString DirectoryString;
	const UObject* WatcherDelegate = WatcherPtr->Delegate;

	//Wait while the runnable pointer hasn't been set

	TotalWatchers++;
	UE_LOG(LogTemp, Log, TEXT("\nStarting Watcher loop %d...\n"), TotalWatchers);

	while (WatcherPtr->ShouldRun)	//Watcher.Runnable->Finished == false
	{
		// Wait for notification.
		//UE_LOG(LogTemp, Log, TEXT("\nWaiting for notification...\n"));

		dwWaitStatus = WaitForMultipleObjects(2, dwChangeHandles,
			FALSE, INFINITE);

		if (!WatcherPtr->ShouldRun)
		{
			UE_LOG(LogTemp, Log, TEXT("\nStop called while sleeping\n"));
			break;
		}
		if (!WatcherDelegate->IsValidLowLevel())
		{
			UE_LOG(LogTemp, Warning, TEXT("\nInvalid Watcher Delegate, exiting watch\n"));
			break;
		}

		switch (dwWaitStatus)
		{
		case WAIT_OBJECT_0:

			ReadDirectoryChangesW(dwChangeHandles[0], (LPVOID)&strFileNotifyInfo, sizeof(strFileNotifyInfo), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE, &dwBytesReturned, NULL, NULL);
			//UE_LOG(LogTemp, Warning, TEXT("Received info about: %s"), strFileNotifyInfo->FileName);

			FileString = FString(strFileNotifyInfo[0].FileNameLength, strFileNotifyInfo[0].FileName);

			// A file was created, renamed, or deleted in the directory.
			// Refresh this directory and restart the notification.

			

			FLambdaRunnable::RunShortLambdaOnGameThread([FullPath, FileString, WatcherDelegate]()
			{
				if (WatcherDelegate->GetClass()->ImplementsInterface(UFolderWatchInterface::StaticClass()))
				{
					FString FilePath = FString::Printf(TEXT("%s\\%s"), *FullPath, *FileString);
					((IFolderWatchInterface*)WatcherDelegate)->Execute_OnFileChanged((UObject*)WatcherDelegate, FileString, FilePath);
				}
			});
			if (FindNextChangeNotification(dwChangeHandles[0]) == FALSE)
			{
				UE_LOG(LogTemp, Warning, TEXT("\n ERROR: FindNextChangeNotification function failed.\n"));
				return;
			}
			break;

		case WAIT_OBJECT_0 + 1:

			// A directory was created, renamed, or deleted.
			// Refresh the tree and restart the notification.

			ReadDirectoryChangesW(dwChangeHandles[1], (LPVOID)&strFileNotifyInfo, sizeof(strFileNotifyInfo), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE, &dwBytesReturned, NULL, NULL);
			DirectoryString = FString(strFileNotifyInfo[0].FileNameLength, strFileNotifyInfo[0].FileName);

			FLambdaRunnable::RunShortLambdaOnGameThread([FullPath, WatcherDelegate, DirectoryString]()
			{
				if (WatcherDelegate->GetClass()->ImplementsInterface(UFolderWatchInterface::StaticClass()))
				{
					FString ChangedDirectoryPath = FString::Printf(TEXT("%s\\%s"), *FullPath, *DirectoryString);
					((IFolderWatchInterface*)WatcherDelegate)->Execute_OnDirectoryChanged((UObject*)WatcherDelegate, DirectoryString, ChangedDirectoryPath);
				}
			});
			if (FindNextChangeNotification(dwChangeHandles[1]) == FALSE)
			{
				UE_LOG(LogTemp, Warning, TEXT("\n ERROR: FindNextChangeNotification function failed.\n"));
				return;
			}
			break;

		case WAIT_TIMEOUT:

			// A timeout occurred, this would happen if some value other 
			// than INFINITE is used in the Wait call and no changes occur.
			// In a single-threaded environment you might not want an
			// INFINITE wait.

			UE_LOG(LogTemp, Warning, TEXT("\nNo changes in the timeout period.\n"));
			break;

		default:
			UE_LOG(LogTemp, Warning, TEXT("\n ERROR: Unhandled dwWaitStatus.\n"));
			return;
			break;
		}
	}

	TotalWatchers--;
	UE_LOG(LogTemp, Log, TEXT("\n Watcher loop stopped, total now: %d.\n"), TotalWatchers);
}

#include "HideWindowsPlatformTypes.h"

#endif